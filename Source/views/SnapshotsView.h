/// @file SnapshotsView.h
/// @brief The row of ten snapshot slots and the glide time

#pragma once

#include <ISnapshotsPresenter.h>
#include <ISnapshotsView.h>
#include <SnapshotSlotView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

/// @brief Snapshot region of the main window
class SnapshotsView
    : public juce::Component
    , public ISnapshotsView
{
public:
    SnapshotsView();

    void setPresenter(ISnapshotsPresenter *presenter)
    {
        mPresenter = presenter;
    }

    void setThumbnailOutline(const std::vector<MapPoint> &frames) override;
    void showSlot(int slot, bool filled, std::optional<MapPoint> at) override;
    void showGlideMs(int milliseconds) override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    ISnapshotsPresenter             *mPresenter = nullptr;
    std::array<SnapshotSlotView, 10> mSlots;
    juce::Image                      mOutline;
    juce::Slider                     mGlide;
};
