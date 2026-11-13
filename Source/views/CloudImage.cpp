/// @file CloudImage.cpp
/// @brief Implementation of CloudImage

#include <CloudImage.h>

#include <Theme.h>

#include <algorithm>

namespace
{
constexpr float kDotSize      = 2.0f;
constexpr float kSegmentWidth = 0.7f;

bool inside(MapPoint p)
{
    return p.x >= 0.0f && p.x <= 1.0f && p.y >= 0.0f && p.y <= 1.0f;
}

juce::uint8 addChannel(juce::uint8 current, float amount)
{
    return static_cast<juce::uint8>(std::min(255.0f, static_cast<float>(current) + amount));
}
} // namespace

void CloudImage::setLayout(std::vector<MapPoint> frames)
{
    mFrames = std::move(frames);
    mDots.clear();
    mSegments.clear();
    mDirty = true;
}

void CloudImage::setAlphas(std::vector<float> dots, std::vector<float> segments)
{
    mDots     = std::move(dots);
    mSegments = std::move(segments);
    mDirty    = true;
}

void CloudImage::clear()
{
    setLayout({});
}

const juce::Image &CloudImage::imageFor(juce::Rectangle<int> size, float scale)
{
    if (mDirty || size != mRenderedSize || scale != mRenderedScale)
    {
        render(size, scale);
    }
    return mImage;
}

juce::Image CloudImage::renderOutline(const std::vector<MapPoint> &frames, int sizePx)
{
    constexpr float kOutlineAlpha = 0.02f;
    CloudImage      outline;
    outline.setLayout(frames);
    outline.setAlphas(std::vector<float>(frames.size(), kOutlineAlpha), {});
    outline.render({0, 0, sizePx, sizePx}, 1.0f);
    return outline.mImage;
}

void CloudImage::render(juce::Rectangle<int> size, float scale)
{
    const int width  = std::max(1, juce::roundToInt(static_cast<float>(size.getWidth()) * scale));
    const int height = std::max(1, juce::roundToInt(static_cast<float>(size.getHeight()) * scale));
    mImage           = juce::Image(juce::Image::ARGB, width, height, true);
    if (!isEmpty())
    {
        {
            juce::Graphics g(mImage);
            drawSegments(g, static_cast<float>(width), static_cast<float>(height));
        }
        addDots(mImage, static_cast<float>(width), static_cast<float>(height), scale);
    }
    mRenderedSize  = size;
    mRenderedScale = scale;
    mDirty         = false;
}

void CloudImage::drawSegments(juce::Graphics &g, float width, float height) const
{
    for (std::size_t f = 0; f < mSegments.size() && f + 1 < mFrames.size(); ++f)
    {
        const auto a = mFrames[f], b = mFrames[f + 1];
        if (mSegments[f] <= 0.0f || !inside(a) || !inside(b))
        {
            continue;
        }
        g.setColour(Theme::cloud.withAlpha(mSegments[f]));
        g.drawLine(
            a.x * width,
            (1.0f - a.y) * height,
            b.x * width,
            (1.0f - b.y) * height,
            kSegmentWidth * width / 360.0f
        );
    }
}

void CloudImage::addDots(juce::Image &image, float width, float height, float scale) const
{
    const juce::Image::BitmapData pixels(image, juce::Image::BitmapData::readWrite);
    const int                     dot = std::max(1, juce::roundToInt(kDotSize * scale));
    for (std::size_t f = 0; f < mFrames.size(); ++f)
    {
        const auto p = mFrames[f];
        if (mDots[f] <= 0.0f || !inside(p))
        {
            continue;
        }
        const int x0 = juce::roundToInt(p.x * width) - (dot / 2);
        const int y0 = juce::roundToInt((1.0f - p.y) * height) - (dot / 2);
        for (int y = std::max(0, y0); y < std::min(image.getHeight(), y0 + dot); ++y)
        {
            for (int x = std::max(0, x0); x < std::min(image.getWidth(), x0 + dot); ++x)
            {
                auto       *pixel = reinterpret_cast<juce::PixelARGB *>(pixels.getPixelPointer(x, y));
                const float a     = mDots[f];
                pixel->setARGB(
                    addChannel(pixel->getAlpha(), 255.0f * a),
                    addChannel(pixel->getRed(), Theme::cloud.getRed() * a),
                    addChannel(pixel->getGreen(), Theme::cloud.getGreen() * a),
                    addChannel(pixel->getBlue(), Theme::cloud.getBlue() * a)
                );
            }
        }
    }
}
