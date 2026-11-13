/// @file MapMarks.cpp
/// @brief Implementation of MapMarks

#include <MapMarks.h>

#include <StringDefinitions.h>
#include <Theme.h>

namespace
{
constexpr float kRingRadius = 9.5f;
constexpr float kPinRadius  = 7.0f;
constexpr float kGaugeWidth = 46.0f;

juce::Point<float> keptInside(juce::Point<float> at, juce::Rectangle<float> area, float margin)
{
    return {
        juce::jlimit(area.getX() + margin, area.getRight() - margin, at.x),
        juce::jlimit(area.getY() + margin, area.getBottom() - margin, at.y)
    };
}
} // namespace

void MapMarks::ring(juce::Graphics &g, juce::Point<float> at, bool offMap, juce::Rectangle<float> area)
{
    const auto centre = keptInside(at, area, kRingRadius + 2.0f);
    g.setColour(Theme::you.withAlpha(0.16f));
    g.drawVerticalLine(juce::roundToInt(centre.x), area.getY(), area.getBottom());
    g.drawHorizontalLine(juce::roundToInt(centre.y), area.getX(), area.getRight());

    juce::Path circle;
    circle.addEllipse(juce::Rectangle<float>(2 * kRingRadius, 2 * kRingRadius).withCentre(centre));
    g.setColour(Theme::you);
    if (offMap)
    {
        const float dashes[] {3.0f, 3.0f};
        juce::Path  dashed;
        juce::PathStrokeType(2.0f).createDashedStroke(dashed, circle, dashes, 2);
        g.fillPath(dashed);
    }
    else
    {
        g.strokePath(circle, juce::PathStrokeType(2.0f));
    }
    g.fillEllipse(juce::Rectangle<float>(4.8f, 4.8f).withCentre(centre));
}

void MapMarks::pin(juce::Graphics &g, juce::Point<float> at, int slot, juce::Rectangle<float> area)
{
    const auto box =
        juce::Rectangle<float>(2 * kPinRadius, 2 * kPinRadius).withCentre(keptInside(at, area, kPinRadius + 3.0f));
    g.setColour(Theme::background);
    g.fillRoundedRectangle(box, 3.0f);
    g.setColour(Theme::text.withAlpha(0.75f));
    g.drawRoundedRectangle(box, 3.0f, 1.0f);
    g.setColour(Theme::text);
    g.setFont(Theme::mono(9.0f).boldened());
    g.drawText(juce::String((slot + 1) % 10), box, juce::Justification::centred);
}

void MapMarks::nearness(
    juce::Graphics &g, juce::Rectangle<float> line, const juce::String &text, float gauge, juce::Colour colour
)
{
    g.setColour(colour);
    g.setFont(Theme::font(11.5f));
    g.drawText(text, line, juce::Justification::centredLeft);
    const float textWidth = static_cast<float>(juce::GlyphArrangement::getStringWidthInt(Theme::font(11.5f), text));
    const auto  track =
        juce::Rectangle<float>(line.getX() + textWidth + 8.0f, line.getCentreY() - 2.0f, kGaugeWidth, 4.0f);
    g.setColour(juce::Colour(0xff'1e'22'27));
    g.fillRoundedRectangle(track, 2.0f);
    g.setColour(colour);
    g.fillRoundedRectangle(track.withWidth(kGaugeWidth * gauge), 2.0f);
}

void MapMarks::legend(juce::Graphics &g, juce::Rectangle<float> area, MapLayer layer)
{
    namespace Ui    = Strings::Ui;
    auto       rows = area;
    const auto row  = [&rows] {
        return rows.removeFromTop(18.0f);
    };
    g.setFont(Theme::font(11.5f));
    const auto scale = [&g](juce::Rectangle<float> line, juce::Colour from, juce::Colour to, const char *label) {
        const auto chip = line.removeFromLeft(14.0f).withSizeKeepingCentre(14.0f, 9.0f);
        g.setGradientFill(juce::ColourGradient::horizontal(from, chip.getX(), to, chip.getRight()));
        g.fillRoundedRectangle(chip, 2.0f);
        g.setColour(Theme::muted);
        g.drawText(Theme::utf8(label), line.withTrimmedLeft(6.0f), juce::Justification::centredLeft);
    };
    if (layer == MapLayer::sound)
    {
        scale(row(), Theme::plot, Theme::sweepNoise, Ui::LegendQuietLoud);
        scale(row(), Theme::sweepNoise, Theme::tonal, Ui::LegendNoiseTone);
    }

    auto cloud = row();
    g.setGradientFill(
        juce::ColourGradient::horizontal(
            Theme::cloud.withAlpha(0.15f),
            cloud.getX(),
            Theme::cloud.withAlpha(0.75f),
            cloud.getX() + 14.0f
        )
    );
    g.fillRoundedRectangle(cloud.removeFromLeft(14.0f).withSizeKeepingCentre(14.0f, 9.0f), 2.0f);
    g.setColour(Theme::muted);
    g.drawText(Theme::utf8(Ui::LegendCloud), cloud.withTrimmedLeft(6.0f), juce::Justification::centredLeft);

    auto you = row();
    g.setColour(Theme::you);
    g.drawEllipse(you.removeFromLeft(14.0f).withSizeKeepingCentre(11.0f, 11.0f), 2.0f);
    g.setColour(Theme::muted);
    g.drawText(Ui::LegendYou, you.withTrimmedLeft(6.0f), juce::Justification::centredLeft);

    auto       snapshot = row();
    const auto icon     = snapshot.removeFromLeft(14.0f).withSizeKeepingCentre(12.0f, 12.0f);
    g.setColour(Theme::text.withAlpha(0.75f));
    g.drawRoundedRectangle(icon, 3.0f, 1.0f);
    g.setColour(Theme::text);
    g.setFont(Theme::mono(8.0f).boldened());
    g.drawText("1", icon, juce::Justification::centred);
    g.setFont(Theme::font(11.5f));
    g.setColour(Theme::muted);
    g.drawText(Ui::LegendSnapshot, snapshot.withTrimmedLeft(6.0f), juce::Justification::centredLeft);
}

void MapMarks::glideLine(juce::Graphics &g, juce::Point<float> from, juce::Point<float> to)
{
    const float dashes[] {2.0f, 3.0f};
    g.setColour(Theme::you.withAlpha(0.7f));
    g.drawDashedLine({from, to}, dashes, 2, 1.2f);
}

void MapMarks::verticalLabel(juce::Graphics &g, const juce::String &text, juce::Rectangle<float> area)
{
    const juce::Graphics::ScopedSaveState saved(g);
    g.addTransform(
        juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, area.getX() + 10.0f, area.getCentreY())
    );
    g.setColour(Theme::dim);
    g.setFont(Theme::font(11.0f));
    g.drawText(
        text,
        juce::Rectangle<float>(area.getX() + 10.0f - 60.0f, area.getCentreY() - 8.0f, 120.0f, 16.0f),
        juce::Justification::centred
    );
}
