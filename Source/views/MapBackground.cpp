/// @file MapBackground.cpp
/// @brief Implementation of MapBackground

#include <MapBackground.h>

#include <Theme.h>

#include <cmath>

namespace
{
constexpr float kCloudOverSound = 0.6f; ///< The piece's cloud stays readable but quiet over the sound
constexpr float kQuietMix       = 0.04f;
constexpr float kLoudMix        = 0.5f; ///< At most half way to the hue, so the marks and texts stay readable
constexpr int   kSmoothing      = 8;    ///< Image pixels per cell side

juce::Colour cellColour(float brightness, float tonal)
{
    const auto hue = Theme::sweepNoise.interpolatedWith(Theme::tonal, tonal);
    return Theme::plot.interpolatedWith(hue, kQuietMix + ((kLoudMix - kQuietMix) * brightness));
}

/// Value at (u, v) in cells, bilinear between cell centres; cells row by row from the bottom
float bilinear(const std::vector<float> &cells, int size, float u, float v)
{
    const auto at = [&](int column, int row) {
        return cells[static_cast<std::size_t>(
            (juce::jlimit(0, size - 1, row) * size) + juce::jlimit(0, size - 1, column)
        )];
    };
    const int   c      = static_cast<int>(std::floor(u));
    const int   r      = static_cast<int>(std::floor(v));
    const float fu     = u - static_cast<float>(c);
    const float fv     = v - static_cast<float>(r);
    const float bottom = (at(c, r) * (1.0f - fu)) + (at(c + 1, r) * fu);
    const float top    = (at(c, r + 1) * (1.0f - fu)) + (at(c + 1, r + 1) * fu);
    return (bottom * (1.0f - fv)) + (top * fv);
}
} // namespace

void MapBackground::setPlain(bool plain)
{
    mPlain = plain;
    if (plain)
    {
        mCloud.clear();
        mSound = {};
    }
}

void MapBackground::setLayer(MapLayer layer)
{
    mLayer = layer;
}

void MapBackground::setCloudLayout(std::vector<MapPoint> frames)
{
    mCloud.setLayout(std::move(frames));
}

void MapBackground::setCloudAlphas(std::vector<float> dots, std::vector<float> segments)
{
    mCloud.setAlphas(std::move(dots), std::move(segments));
}

void MapBackground::setSound(std::size_t size, const std::vector<float> &brightness, const std::vector<float> &tonal)
{
    if (size == 0 || brightness.size() != size * size || tonal.size() != size * size)
    {
        mSound = {};
        return;
    }
    // Smoothed here rather than by stretching: image scaling may not interpolate.
    const auto cells = static_cast<int>(size);
    const auto side  = cells * kSmoothing;
    mSound           = juce::Image(juce::Image::RGB, side, side, false);
    for (int y = 0; y < side; ++y)
    {
        const float v = ((static_cast<float>(side - 1 - y) + 0.5f) / kSmoothing) - 0.5f; // cells from the bottom
        for (int x = 0; x < side; ++x)
        {
            const float u = ((static_cast<float>(x) + 0.5f) / kSmoothing) - 0.5f;
            mSound.setPixelAt(x, y, cellColour(bilinear(brightness, cells, u, v), bilinear(tonal, cells, u, v)));
        }
    }
}

void MapBackground::paint(juce::Graphics &g, juce::Rectangle<float> area, juce::Point<float> origin, float scale)
{
    g.setColour(Theme::plot);
    g.fillRoundedRectangle(area, kCorner);
    if (mPlain)
    {
        paintPlainGrid(g, area);
        return;
    }
    const bool sound = mLayer == MapLayer::sound;
    if (sound)
    {
        paintSound(g, area);
    }
    g.setColour(Theme::axisCross);
    g.drawVerticalLine(juce::roundToInt(origin.x), area.getY(), area.getBottom());
    g.drawHorizontalLine(juce::roundToInt(origin.y), area.getX(), area.getRight());
    g.setOpacity(sound ? kCloudOverSound : 1.0f);
    g.drawImage(mCloud.imageFor(area.toNearestInt().withZeroOrigin(), scale), area);
    g.setOpacity(1.0f);
}

void MapBackground::paintPlainGrid(juce::Graphics &g, juce::Rectangle<float> area) const
{
    g.setColour(Theme::axisCross);
    for (int i = 1; i < 4; ++i)
    {
        g.drawVerticalLine(juce::roundToInt(area.getX() + (area.getWidth() * i / 4.0f)), area.getY(), area.getBottom());
        g.drawHorizontalLine(
            juce::roundToInt(area.getY() + (area.getHeight() * i / 4.0f)),
            area.getX(),
            area.getRight()
        );
    }
}

void MapBackground::paintSound(juce::Graphics &g, juce::Rectangle<float> area) const
{
    if (!mSound.isValid())
    {
        return;
    }
    // The cells are smoothed into each other when the image is stretched.
    const juce::Graphics::ScopedSaveState state(g);
    juce::Path                            rounded;
    rounded.addRoundedRectangle(area, kCorner);
    g.reduceClipRegion(rounded);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(mSound, area, juce::RectanglePlacement::stretchToFit);
}
