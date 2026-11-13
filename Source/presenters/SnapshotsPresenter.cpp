/// @file SnapshotsPresenter.cpp
/// @brief Implementation of SnapshotsPresenter

#include <SnapshotsPresenter.h>

#include <algorithm>

SnapshotsPresenter::SnapshotsPresenter(LatentState &state, SnapshotBank &bank, ISnapshotsView &view, Clock clock)
    : mState(state)
    , mBank(bank)
    , mView(view)
    , mClock(std::move(clock))
{
    mBank.addListener(this);
    mState.addListener(this);
    mView.showGlideMs(mGlideMs);
    snapshotsChanged(mBank);
}

SnapshotsPresenter::~SnapshotsPresenter()
{
    mState.removeListener(this);
    mBank.removeListener(this);
}

void SnapshotsPresenter::setModel(std::shared_ptr<const TrackAnalysis> analysis, const ControlRanges &ranges)
{
    stopGlide();
    mThumbnailSpace.reset();
    if (analysis)
    {
        mThumbnailSpace.emplace(analysis, MapAxes {MapAxes::Space::principal, 0, 1}, ranges);
    }
    mView.setThumbnailOutline(mThumbnailSpace ? mThumbnailSpace->frameLayout() : std::vector<MapPoint> {});
}

void SnapshotsPresenter::onDigit(int slot, bool shift)
{
    shift ? onSlotHeld(slot) : onSlotTapped(slot);
}

void SnapshotsPresenter::onFrameTick()
{
    if (const auto value = mGlide.tick(mClock()))
    {
        mGlided        = *value;
        mApplyingGlide = true;
        mState.setLatent(*value);
        mApplyingGlide = false;
        if (!mGlide.isRunning())
        {
            stopGlide();
        }
    }
}

void SnapshotsPresenter::onSlotTapped(int slot)
{
    const auto &z = mBank.at(static_cast<std::size_t>(slot));
    if (!z || z->size() != mState.latent().size())
    {
        return;
    }
    mGlide.start(mState.latent(), *z, mGlideMs, mClock());
    if (onGlideTarget)
    {
        onGlideTarget(*z);
    }
    onFrameTick(); // a glide of 0 ms lands at once
}

void SnapshotsPresenter::onSlotHeld(int slot)
{
    if (!mState.latent().empty())
    {
        mBank.store(static_cast<std::size_t>(slot), mState.latent());
    }
}

void SnapshotsPresenter::onGlideChanged(int milliseconds)
{
    mGlideMs = std::clamp(milliseconds, 0, kMaxGlideMs);
}

void SnapshotsPresenter::snapshotsChanged(const SnapshotBank &bank)
{
    for (std::size_t slot = 0; slot < SnapshotBank::kSlots; ++slot)
    {
        const auto             &z = bank.at(slot);
        std::optional<MapPoint> at;
        if (z && mThumbnailSpace && z->size() == mState.ranges().z.size())
        {
            at = mThumbnailSpace->place(mThumbnailSpace->coordsOf(*z)).at;
        }
        mView.showSlot(static_cast<int>(slot), z.has_value(), at);
    }
}

void SnapshotsPresenter::latentStateChanged(const LatentState &state)
{
    if (mGlide.isRunning() && !mApplyingGlide && state.latent() != mGlided)
    {
        stopGlide(); // someone else moved z: they win
    }
}

void SnapshotsPresenter::stopGlide()
{
    mGlide.cancel();
    if (onGlideTarget)
    {
        onGlideTarget(std::nullopt);
    }
}
