/// @file MainPresenter.cpp
/// @brief Implementation of MainPresenter

#include <MainPresenter.h>

#include <ControlRanges.h>
#include <StartingLatent.h>

#include <algorithm>

namespace
{
/// Map A plots axes 0 and 1, map B 2 and 3 (or the last dims of a small model)
MapAxes axesFor(int map, std::size_t latentDim, bool hasTrack)
{
    const auto last  = latentDim > 0 ? latentDim - 1 : 0;
    const auto first = std::min<std::size_t>(2U * static_cast<std::size_t>(map), last > 0 ? last - 1 : 0);
    return {hasTrack ? MapAxes::Space::principal : MapAxes::Space::raw, first, std::min(first + 1, last)};
}
} // namespace

MainPresenter::MainPresenter(LatentState &state, SnapshotBank &bank, MainViews views, SweepScheduler::Clock clock)
    : mState(state)
    , mBank(bank)
    , mSweeps(clock)
    , mTopBar(views.topBar)
    , mOutput(state, views.output, clock)
    , mMapA(state, bank, views.mapA, 0, clock)
    , mMapB(state, bank, views.mapB, 1, clock)
    , mLatentPanel(state, views.latent, mSweeps)
    , mSnapshots(state, bank, views.snapshots, clock)
{
    mSnapshots.onGlideTarget = [this](const std::optional<std::vector<float>> &target) {
        mMapA.showGlideTarget(target);
        mMapB.showGlideTarget(target);
    };
    const auto learnFrom = [this](MidiTarget target) {
        learn(target);
    };
    mMapA.onLearnRequested        = learnFrom;
    mMapB.onLearnRequested        = learnFrom;
    mOutput.onLearnRequested      = learnFrom;
    mLatentPanel.onLearnRequested = learnFrom;
    mTopBar.onMuteChanged         = [this](bool muted) {
        if (onMuteChanged)
        {
            onMuteChanged(muted);
        }
    };
    mTopBar.onMidiMapToggleRequested = [this] {
        toggleMidiMapMode();
    };
}

void MainPresenter::setModel(
    const ModelDescriptor &descriptor, std::shared_ptr<const LatentTrack> track, BackgroundSubmitters submitters
)
{
    const auto ranges = ControlRanges::forModel(descriptor);
    mAnalysis         = track ? std::make_shared<const TrackAnalysis>(track, descriptor.zRange) : nullptr;
    mSweeps.setSubmitter(std::move(submitters.sweeps));
    for (const std::size_t map : {0U, 1U})
    {
        auto submitter = submitters.soundGrids ? [submit = submitters.soundGrids, map](const SoundGridJob &job) {
            submit(map, job);
        } : SoundMapLayer::Scheduler::Submitter {};
        (map == 0 ? mMapA : mMapB).setSoundSubmitter(std::move(submitter));
    }

    mTopBar.setModel(descriptor);
    mOutput.setModel(descriptor);
    mLatentPanel.setModel(mAnalysis, ranges);
    mSnapshots.setModel(mAnalysis, ranges);
    mMapA.setModel(mAnalysis, axesFor(0, descriptor.latentDim, mAnalysis != nullptr), ranges);
    mMapB.setModel(mAnalysis, axesFor(1, descriptor.latentDim, mAnalysis != nullptr), ranges);

    refreshCcTags();
    mBank.clear();
    mState.reset(ranges, startingLatent(ranges, track.get()), descriptor.defaultXMax, descriptor.defaultSClip);
}

void MainPresenter::onSpectrum(std::span<const float> bins, float rms)
{
    mOutput.onSpectrum(bins, rms);
}

void MainPresenter::onFrameTick()
{
    mSnapshots.onFrameTick();
    mSweeps.tick();
    mMapA.onFrameTick();
    mMapB.onFrameTick();
}

void MainPresenter::onDigit(int slot, bool shift)
{
    mSnapshots.onDigit(slot, shift);
}

void MainPresenter::setAudioStatus(double sampleRate, int bufferSize)
{
    mTopBar.setAudioStatus(sampleRate, bufferSize);
    mOutput.setDeviceSampleRate(sampleRate);
}

void MainPresenter::setMidiStatus(const std::string &device, std::optional<int> lastController)
{
    mTopBar.setMidiStatus(device, lastController);
}

void MainPresenter::onSweeps(const SweepResult &result)
{
    mLatentPanel.onSweepResult(result);
}

void MainPresenter::onSoundGrid(std::size_t map, const SoundGridResult &result)
{
    (map == 0 ? mMapA : mMapB).onSoundGrid(result);
}

void MainPresenter::setMidiAssignments(IMidiAssignments *assignments)
{
    mAssignments = assignments;
    refreshCcTags();
}

void MainPresenter::toggleMidiMapMode()
{
    mMidiMapMode = !mMidiMapMode;
    if (!mMidiMapMode && mAssignments)
    {
        mAssignments->cancelLearning();
    }
    refreshCcTags();
}

void MainPresenter::refreshCcTags()
{
    const auto *assignments = mMidiMapMode ? mAssignments : nullptr;
    mTopBar.showMidiMapMode(mMidiMapMode);
    mOutput.showCcTags(assignments);
    mMapA.showCcTags(assignments);
    mMapB.showCcTags(assignments);
    mLatentPanel.showCcTags(assignments);
}

void MainPresenter::learn(MidiTarget target)
{
    if (!mMidiMapMode || !mAssignments)
    {
        return;
    }
    mAssignments->learning() == target ? mAssignments->cancelLearning() : mAssignments->beginLearning(target);
    refreshCcTags();
}

void MainPresenter::setMapAxisFromMidi(size_t map, size_t axis, float value)
{
    (map == 0 ? mMapA : mMapB).setAxisFromMidi(axis, value);
}

void MainPresenter::setLatentFromMidi(size_t index, float value)
{
    mState.setZ(index, value);
}

void MainPresenter::setXMaxFromMidi(float value)
{
    mState.setXMax(value);
}

void MainPresenter::setSClipFromMidi(float value)
{
    mState.setSClip(value);
}
