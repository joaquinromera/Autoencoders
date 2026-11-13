/// @file SpectrumPlot.cpp
/// @brief Implementation of SpectrumPlot

#include <SpectrumPlot.h>

#include <StringDefinitions.h>
#include <Theme.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr int   kGridStepDb = 20;
constexpr float kLeft = 40.0f, kRight = 46.0f, kTop = 10.0f, kBottom = 18.0f;
constexpr float kGrabPixels = 6.0f;
constexpr float kMeterLowDb = -60.0f;

const std::vector<std::pair<double, const char *>> kFrequencyTicks {
    {   100.0, "100"},
    {   200.0, "200"},
    {   500.0, "500"},
    { 1'000.0,  "1k"},
    { 2'000.0,  "2k"},
    { 5'000.0,  "5k"},
    {10'000.0, "10k"}
};
} // namespace

void SpectrumPlot::setFrequencies(std::vector<double> hz)
{
    mFrequencies = std::move(hz);
    mSpectrum.clear();
    repaint();
}

void SpectrumPlot::setSpectrum(const std::vector<float> &db)
{
    mSpectrum = db;
    repaint();
}

void SpectrumPlot::setBand(const OutputBand &band)
{
    mBand = band;
    repaint();
}

void SpectrumPlot::setMeter(float levelDb, float peakDb)
{
    mMeterDb = levelDb;
    mPeakDb  = peakDb;
    repaint();
}

juce::Rectangle<float> SpectrumPlot::plotArea() const
{
    return getLocalBounds()
        .toFloat()
        .withTrimmedLeft(kLeft)
        .withTrimmedRight(kRight)
        .withTrimmedTop(kTop)
        .withTrimmedBottom(kBottom);
}

float SpectrumPlot::yOf(float db) const
{
    const auto  area = plotArea();
    const float span = std::max(1.0f, mBand.highestDb - mBand.lowestDb);
    return area.getY() + (area.getHeight() * (1.0f - ((db - mBand.lowestDb) / span)));
}

float SpectrumPlot::dbOf(float y) const
{
    const auto area = plotArea();
    return mBand.highestDb - ((y - area.getY()) / area.getHeight() * (mBand.highestDb - mBand.lowestDb));
}

float SpectrumPlot::xOfHz(double hz) const
{
    const auto area = plotArea();
    if (mFrequencies.size() < 2)
    {
        return area.getX();
    }
    const double t = std::log(hz / mFrequencies.front()) / std::log(mFrequencies.back() / mFrequencies.front());
    return area.getX() + (area.getWidth() * static_cast<float>(t));
}

void SpectrumPlot::paint(juce::Graphics &g)
{
    const auto area = plotArea();
    g.setColour(Theme::plot);
    g.fillRect(area);
    paintGrid(g);
    g.setColour(Theme::text.withAlpha(0.045f));
    g.fillRect(area.withTop(yOf(mBand.floorDb + mBand.rangeDb)).withBottom(yOf(mBand.floorDb)));
    paintSpectrum(g);
    g.setColour(Theme::line);
    g.drawRect(area);
    paintBand(g); // over the border, so a line at a limit stays visible
    paintMeter(g);
}

void SpectrumPlot::paintGrid(juce::Graphics &g) const
{
    const auto area = plotArea();
    g.setFont(Theme::mono(9.5f));
    // The bottom edge is the lowest floor, so it gets a label of its own.
    const int lowest = static_cast<int>(std::ceil(mBand.lowestDb / kGridStepDb)) * kGridStepDb;
    for (int db = lowest; static_cast<float>(db) < mBand.highestDb; db += kGridStepDb)
    {
        g.setColour(Theme::grid);
        g.drawHorizontalLine(static_cast<int>(yOf(static_cast<float>(db))), area.getX(), area.getRight());
        g.setColour(Theme::dim);
        const auto label = db > 0 ? "+" + juce::String(db) : juce::String(db).replace("-", juce::String::fromUTF8("−"));
        g.drawText(
            label,
            juce::Rectangle<float>(0.0f, yOf(static_cast<float>(db)) - 6.0f, kLeft - 6.0f, 12.0f),
            juce::Justification::centredRight
        );
    }
    for (const auto &[hz, label] : kFrequencyTicks)
    {
        if (mFrequencies.size() < 2 || hz > mFrequencies.back())
        {
            continue;
        }
        g.setColour(Theme::axisCross);
        g.drawVerticalLine(static_cast<int>(xOfHz(hz)), area.getY(), area.getBottom());
        g.setColour(Theme::dim);
        g.drawText(
            label,
            juce::Rectangle<float>(xOfHz(hz) - 20.0f, area.getBottom() + 2.0f, 40.0f, 14.0f),
            juce::Justification::centred
        );
    }
}

void SpectrumPlot::paintSpectrum(juce::Graphics &g) const
{
    if (mSpectrum.size() < 2 || mSpectrum.size() != mFrequencies.size())
    {
        return;
    }
    const auto area = plotArea();
    juce::Path line;
    for (std::size_t k = 0; k < mSpectrum.size(); ++k)
    {
        const juce::Point<float> point {
            xOfHz(mFrequencies[k]),
            yOf(juce::jlimit(mBand.lowestDb, mBand.highestDb, mSpectrum[k]))
        };
        k == 0 ? line.startNewSubPath(point) : line.lineTo(point);
    }
    auto fill = line;
    fill.lineTo(area.getRight(), area.getBottom());
    fill.lineTo(area.getX(), area.getBottom());
    fill.closeSubPath();
    g.setColour(Theme::you.withAlpha(0.08f));
    g.fillPath(fill);
    g.setColour(Theme::you);
    g.strokePath(line, juce::PathStrokeType(1.4f));
}

void SpectrumPlot::paintBand(juce::Graphics &g) const
{
    const auto area = plotArea();
    g.setFont(Theme::mono(9.5f));
    const std::pair<float, const char *> lines[] {
        {mBand.floorDb + mBand.rangeDb, Strings::Ui::CeilingLine},
        {                mBand.floorDb,   Strings::Ui::FloorLine}
    };
    for (const auto &[db, label] : lines)
    {
        // The pixel row stays inside the plot, on the border row at either limit.
        const int row = juce::jlimit(
            juce::roundToInt(area.getY()),
            juce::roundToInt(area.getBottom()) - 1,
            static_cast<int>(yOf(db))
        );
        const float y = static_cast<float>(row) + 0.5f; // the row's centre
        g.setColour(Theme::bandLine);
        g.drawHorizontalLine(row, area.getX(), area.getRight());
        g.setColour(Theme::text);
        g.fillRoundedRectangle(area.getCentreX() - 11.0f, y - 3.0f, 22.0f, 6.0f, 3.0f);
        g.setColour(Theme::muted);
        // Both labels sit inside the band, so neither leaves the plot at its limit.
        const bool ceiling = label == Strings::Ui::CeilingLine;
        g.drawText(
            label,
            juce::Rectangle<float>(area.getX() + 8.0f, ceiling ? y + 4.0f : y - 16.0f, area.getWidth() - 16.0f, 12.0f),
            ceiling ? juce::Justification::centredRight : juce::Justification::centredLeft
        );
    }
}

void SpectrumPlot::paintMeter(juce::Graphics &g) const
{
    const auto area = plotArea();
    const auto meter =
        juce::Rectangle<float>(static_cast<float>(getWidth()) - 24.0f, area.getY(), 10.0f, area.getHeight());
    g.setColour(Theme::plot);
    g.fillRoundedRectangle(meter, 2.0f);
    g.setColour(Theme::line);
    g.drawRoundedRectangle(meter, 2.0f, 1.0f);
    const float level = juce::jlimit(0.0f, 1.0f, (mMeterDb - kMeterLowDb) / -kMeterLowDb);
    g.setColour(Theme::meter);
    g.fillRoundedRectangle(meter.reduced(1.5f).withTrimmedTop(meter.getHeight() * (1.0f - level)), 1.5f);
    const float peak = juce::jlimit(0.0f, 1.0f, (mPeakDb - kMeterLowDb) / -kMeterLowDb);
    g.setColour(Theme::text);
    g.fillRect(meter.getX() - 1.0f, meter.getBottom() - (meter.getHeight() * peak), meter.getWidth() + 2.0f, 1.2f);
    g.setColour(Theme::dim);
    g.setFont(Theme::mono(9.5f));
    g.drawText(
        Strings::Ui::Out,
        meter.withY(area.getBottom() + 2.0f).withHeight(14.0f).expanded(10.0f, 0.0f),
        juce::Justification::centred
    );
}

void SpectrumPlot::mouseDown(const juce::MouseEvent &event)
{
    if (onBandPressed)
    {
        onBandPressed(dbOf(event.position.y), dbOf(0.0f) - dbOf(kGrabPixels));
    }
}

void SpectrumPlot::mouseDrag(const juce::MouseEvent &event)
{
    if (onBandDragged)
    {
        onBandDragged(dbOf(event.position.y));
    }
}
