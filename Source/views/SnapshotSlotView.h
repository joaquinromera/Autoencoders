/// @file SnapshotSlotView.h
/// @brief One snapshot slot: click goes there, hold (or shift-click) stores

#pragma once

#include <IMapView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <optional>

/// @brief A slot with its number, a thumbnail of where it lies, and a hold arc
class SnapshotSlotView
    : public juce::Component
    , private juce::Timer
{
public:
    static constexpr int kHoldMs = 500;

    std::function<void()> onTapped;
    std::function<void()> onHeld;

    void setSlot(int slot)
    {
        mSlot = slot;
    }

    void setFilled(bool filled, std::optional<MapPoint> at, const juce::Image &outline);

    void paint(juce::Graphics &g) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    void  timerCallback() override;
    float holdProgress() const;
    void  paintThumbnail(juce::Graphics &g, juce::Rectangle<float> area) const;

    int                     mSlot   = 0;
    bool                    mFilled = false;
    std::optional<MapPoint> mAt;
    juce::Image             mOutline;
    juce::uint32            mPressedAt = 0;
    bool                    mHolding   = false;
};
