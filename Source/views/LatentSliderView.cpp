/// @file LatentSliderView.cpp
/// @brief Implementation of LatentSliderView

#include <LatentSliderView.h>

#include <Theme.h>

namespace
{
constexpr float kLabelHeight    = 16.0f;
constexpr float kStripInset     = 12.0f; ///< Above and below the range inside the strip area
constexpr float kShownZeroShare = 0.05f; ///< Fewer frames at 0 than this are not marked
} // namespace

LatentSliderView::LatentSliderView(juce::String label, float min, float max)
    : mLabel(std::move(label))
    , mMin(min)
    , mMax(max)
{
    mTag.onClicked = [this] {
        if (onCcTagClicked)
        {
            onCcTagClicked();
        }
    };
    addChildComponent(mTag);
}

void LatentSliderView::setCcTag(std::optional<CcTagInfo> tag)
{
    mTag.setVisible(tag.has_value());
    if (tag)
    {
        mTag.setInfo(*tag);
    }
}

void LatentSliderView::resized()
{
    mTag.setBounds((getWidth() - CcTag::kWidth) / 2, static_cast<int>(kLabelHeight) + 2, CcTag::kWidth, CcTag::kHeight);
}

void LatentSliderView::setValue(float value, bool flagged)
{
    mValue   = value;
    mFlagged = flagged;
    repaint();
}

void LatentSliderView::setHistogram(std::vector<float> heights, float zeroShare, bool empty)
{
    mHeights   = std::move(heights);
    mZeroShare = zeroShare;
    mEmpty     = empty;
    repaint();
}

void LatentSliderView::setSweep(std::vector<float> brightness, std::vector<float> tonal)
{
    mSweep = std::move(brightness);
    mTonal = std::move(tonal);
    repaint();
}

void LatentSliderView::setStripsVisible(bool visible)
{
    mStrips = visible;
    repaint();
}

juce::Rectangle<float> LatentSliderView::stripArea() const
{
    return getLocalBounds().toFloat().withTrimmedTop(kLabelHeight).withTrimmedBottom(kLabelHeight);
}

float LatentSliderView::yOf(float value) const
{
    const auto area   = stripArea();
    const auto top    = area.getY() + kStripInset;
    const auto bottom = area.getBottom() - kStripInset;
    return bottom - ((value - mMin) / (mMax - mMin) * (bottom - top));
}

float LatentSliderView::trackX() const
{
    return std::round(static_cast<float>(getWidth()) * 0.42f) + 0.5f;
}

void LatentSliderView::paint(juce::Graphics &g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour(Theme::muted);
    g.setFont(Theme::mono(12.0f));
    g.drawText(mLabel, bounds.withHeight(kLabelHeight), juce::Justification::centred);
    if (mStrips)
    {
        paintSweep(g);
        paintHistogram(g);
        paintZeroShare(g);
    }
    paintTrackAndThumb(g);
    g.setColour(mFlagged ? Theme::warn : Theme::text);
    g.drawText(
        juce::String(mValue, 2),
        bounds.withTrimmedTop(bounds.getHeight() - kLabelHeight),
        juce::Justification::centred
    );
}

void LatentSliderView::paintSweep(juce::Graphics &g) const
{
    if (mSweep.size() < 2 || mTonal.size() != mSweep.size())
    {
        return;
    }
    const auto  n    = static_cast<float>(mSweep.size());
    const float step = (yOf(mMin) - yOf(mMax)) / (n - 1.0f);
    for (std::size_t k = 0; k < mSweep.size(); ++k)
    {
        const float y      = yOf(mMin + (static_cast<float>(k) * (mMax - mMin) / (n - 1.0f)));
        const auto  colour = Theme::sweepNoise.interpolatedWith(Theme::tonal, mTonal[k]);
        g.setColour(colour.withAlpha(0.06f + (0.6f * mSweep[k])));
        g.fillRect(trackX() - 12.0f, y - (step / 2.0f), 6.0f, step + 0.3f);
    }
}

void LatentSliderView::paintHistogram(juce::Graphics &g) const
{
    if (mEmpty || mHeights.empty())
    {
        return;
    }
    const float bottom = yOf(mMin);
    const float height = (bottom - yOf(mMax)) / static_cast<float>(mHeights.size());
    const float room   = static_cast<float>(getWidth()) - trackX() - 8.0f;
    g.setColour(Theme::histogram);
    for (std::size_t k = 0; k < mHeights.size(); ++k)
    {
        if (mHeights[k] > 0.02f)
        {
            g.fillRect(
                trackX() + 4.0f,
                bottom - (static_cast<float>(k + 1) * height),
                mHeights[k] * room,
                height - 0.4f
            );
        }
    }
}

void LatentSliderView::paintZeroShare(juce::Graphics &g) const
{
    if (mEmpty || mZeroShare < kShownZeroShare || mMin > 0.0f || mMax < 0.0f)
    {
        return;
    }
    const float y = yOf(0.0f);
    g.setColour(Theme::cloud);
    g.fillRect(trackX() + 4.0f, y - 1.0f, 5.0f, 2.0f);
    g.setColour(Theme::muted);
    g.setFont(Theme::mono(9.5f));
    const auto label = juce::String(juce::roundToInt(mZeroShare * 100.0f)) + "%";
    g.drawText(
        label,
        juce::Rectangle<float>(trackX() + 10.0f, y - 6.0f, 30.0f, 12.0f),
        juce::Justification::centredLeft
    );
}

void LatentSliderView::paintTrackAndThumb(juce::Graphics &g) const
{
    const juce::Line<float> track {trackX(), yOf(mMax), trackX(), yOf(mMin)};
    const bool              dashed = mStrips && mEmpty;
    g.setColour(dashed ? Theme::emptyTrack : Theme::track);
    if (dashed)
    {
        const float dashes[] {3.0f, 3.0f};
        g.drawDashedLine(track, dashes, 2, 2.0f);
    }
    else
    {
        g.drawLine(track, 2.0f);
    }
    const auto thumb = juce::Rectangle<float>(22.0f, 8.0f).withCentre({trackX(), yOf(mValue)});
    g.setColour(mFlagged ? Theme::warn : Theme::you);
    g.fillRoundedRectangle(thumb, 3.0f);
    g.setColour(Theme::background);
    g.drawRoundedRectangle(thumb, 3.0f, 1.0f);
}

void LatentSliderView::mouseDown(const juce::MouseEvent &event)
{
    mouseDrag(event);
}

void LatentSliderView::mouseDrag(const juce::MouseEvent &event)
{
    if (!onDragged)
    {
        return;
    }
    const float t = (yOf(mMin) - event.position.y) / (yOf(mMin) - yOf(mMax));
    onDragged(mMin + (juce::jlimit(0.0f, 1.0f, t) * (mMax - mMin)));
}
