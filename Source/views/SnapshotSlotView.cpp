/// @file SnapshotSlotView.cpp
/// @brief Implementation of SnapshotSlotView

#include <SnapshotSlotView.h>

#include <Theme.h>

namespace
{
constexpr int   kArcHz         = 30;
constexpr float kThumbnailSize = 36.0f;
} // namespace

void SnapshotSlotView::setFilled(bool filled, std::optional<MapPoint> at, const juce::Image &outline)
{
    mFilled  = filled;
    mAt      = at;
    mOutline = outline;
    repaint();
}

float SnapshotSlotView::holdProgress() const
{
    return mHolding ? juce::jlimit(
                          0.0f,
                          1.0f,
                          static_cast<float>(juce::Time::getMillisecondCounter() - mPressedAt) / kHoldMs
                      )
                    : 0.0f;
}

void SnapshotSlotView::paint(juce::Graphics &g)
{
    auto box = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(mFilled ? Theme::raised : Theme::background);
    g.fillRoundedRectangle(box, 6.0f);
    juce::Path outline;
    outline.addRoundedRectangle(box, 6.0f);
    g.setColour(mFilled ? Theme::line2 : Theme::line);
    if (mFilled)
    {
        g.strokePath(outline, juce::PathStrokeType(1.0f));
    }
    else
    {
        const float dashes[] {4.0f, 3.0f};
        juce::Path  dashed;
        juce::PathStrokeType(1.0f).createDashedStroke(dashed, outline, dashes, 2);
        g.fillPath(dashed);
    }
    if (mFilled && mOutline.isValid())
    {
        paintThumbnail(
            g,
            box.removeFromLeft(kThumbnailSize + 6.0f).withSizeKeepingCentre(kThumbnailSize, kThumbnailSize)
        );
    }
    g.setColour(mFilled ? Theme::text : Theme::dim);
    g.setFont(Theme::mono(11.5f));
    g.drawText(juce::String((mSlot + 1) % 10), box, juce::Justification::centred);

    if (const float progress = holdProgress(); progress > 0.0f)
    {
        juce::Path arc;
        const auto ring = getLocalBounds().toFloat().withSizeKeepingCentre(22.0f, 22.0f);
        arc.addCentredArc(
            ring.getCentreX(),
            ring.getCentreY(),
            11.0f,
            11.0f,
            0.0f,
            0.0f,
            progress * juce::MathConstants<float>::twoPi,
            true
        );
        g.setColour(Theme::you);
        g.strokePath(arc, juce::PathStrokeType(2.0f));
    }
}

void SnapshotSlotView::paintThumbnail(juce::Graphics &g, juce::Rectangle<float> area) const
{
    g.setColour(Theme::plot);
    g.fillRoundedRectangle(area, 3.0f);
    g.drawImage(mOutline, area);
    if (mAt)
    {
        const juce::Point<float> dot {
            area.getX() + (mAt->x * area.getWidth()),
            area.getBottom() - (mAt->y * area.getHeight())
        };
        g.setColour(Theme::text);
        g.fillEllipse(juce::Rectangle<float>(6.4f, 6.4f).withCentre(dot));
    }
}

void SnapshotSlotView::mouseDown(const juce::MouseEvent &event)
{
    if (event.mods.isShiftDown())
    {
        if (onHeld)
        {
            onHeld();
        }
        return;
    }
    mPressedAt = juce::Time::getMillisecondCounter();
    mHolding   = true;
    startTimerHz(kArcHz);
}

void SnapshotSlotView::mouseUp(const juce::MouseEvent &event)
{
    const bool wasHolding = std::exchange(mHolding, false);
    stopTimer();
    repaint();
    if (wasHolding && onTapped && getLocalBounds().contains(event.getPosition()))
    {
        onTapped();
    }
}

void SnapshotSlotView::timerCallback()
{
    if (holdProgress() >= 1.0f)
    {
        mHolding = false;
        stopTimer();
        if (onHeld)
        {
            onHeld();
        }
    }
    repaint();
}
