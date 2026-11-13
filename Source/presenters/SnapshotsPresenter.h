/// @file SnapshotsPresenter.h
/// @brief Stores snapshots of the latent vector and glides back to them

#pragma once

#include <GlideRamp.h>
#include <ISnapshotsPresenter.h>
#include <ISnapshotsView.h>
#include <LatentState.h>
#include <MapSpace.h>
#include <SnapshotBank.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

/// @brief Presenter of the snapshot row
///
/// A tap glides the sliders to the stored z over the glide time; a hold
/// stores the current z. Any change of z the glide did not make (a drag,
/// MIDI) cancels the glide.
class SnapshotsPresenter
    : public ISnapshotsPresenter
    , private SnapshotBank::Listener
    , private LatentState::Listener
{
public:
    static constexpr int kDefaultGlideMs = 250;
    static constexpr int kMaxGlideMs     = 2'000;

    using Clock = std::function<double()>;

    SnapshotsPresenter(LatentState &state, SnapshotBank &bank, ISnapshotsView &view, Clock clock);
    ~SnapshotsPresenter() override;

    SnapshotsPresenter(const SnapshotsPresenter &)            = delete;
    SnapshotsPresenter &operator=(const SnapshotsPresenter &) = delete;

    /// @param analysis Places the thumbnails on map A's principal axes; null: no thumbnails
    void setModel(std::shared_ptr<const TrackAnalysis> analysis, const ControlRanges &ranges);

    /// @brief Digit keys: 1..9, 0 go there; with shift they store
    void onDigit(int slot, bool shift);

    /// @brief Advances a running glide
    void onFrameTick();

    void onSlotTapped(int slot) override;
    void onSlotHeld(int slot) override;
    void onGlideChanged(int milliseconds) override;

    /// @brief Where the running glide goes, or nothing when it stops
    std::function<void(const std::optional<std::vector<float>> &)> onGlideTarget;

private:
    void snapshotsChanged(const SnapshotBank &bank) override;
    void latentStateChanged(const LatentState &state) override;
    void stopGlide();

    LatentState            &mState;
    SnapshotBank           &mBank;
    ISnapshotsView         &mView;
    Clock                   mClock;
    GlideRamp               mGlide;
    std::vector<float>      mGlided; ///< The last z the glide set
    bool                    mApplyingGlide = false;
    int                     mGlideMs       = kDefaultGlideMs;
    std::optional<MapSpace> mThumbnailSpace;
};
