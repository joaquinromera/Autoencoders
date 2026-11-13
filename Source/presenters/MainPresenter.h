/// @file MainPresenter.h
/// @brief Coordinates the window: model changes, engine feedback, MIDI

#pragma once

#include <BackgroundSubmitters.h>
#include <LatentPanelPresenter.h>
#include <MapPresenter.h>
#include <OutputPresenter.h>
#include <SnapshotsPresenter.h>
#include <SweepScheduler.h>
#include <TopBarPresenter.h>

#include <IMidiAssignments.h>
#include <LatentState.h>
#include <LatentTrack.h>
#include <ModelDescriptor.h>
#include <SnapshotBank.h>
#include <TrackAnalysis.h>

#include <functional>
#include <memory>
#include <span>

/// @brief The views of the main window
struct MainViews
{
    ITopBarView      &topBar;
    IOutputView      &output;
    IMapView         &mapA;
    IMapView         &mapB;
    ILatentPanelView &latent;
    ISnapshotsView   &snapshots;
};

/// @brief Owns the region presenters and fans model changes out to them
///
/// The regions share nothing but LatentState, SnapshotBank and the model's
/// immutable TrackAnalysis.
class MainPresenter
{
public:
    MainPresenter(LatentState &state, SnapshotBank &bank, MainViews views, SweepScheduler::Clock clock);

    MainPresenter(const MainPresenter &)            = delete;
    MainPresenter &operator=(const MainPresenter &) = delete;

    /// @brief Shows a newly loaded model and starts it at its starting point
    /// @param track Null when the file has no ztrack: plain pads, no strips
    /// @param submitters Where sweep and sound grid jobs go; either may be empty
    void setModel(
        const ModelDescriptor &descriptor, std::shared_ptr<const LatentTrack> track, BackgroundSubmitters submitters
    );

    /// @brief The engine's last block: decoder output and output level
    void onSpectrum(std::span<const float> bins, float rms);

    /// @brief Called regularly by the GUI timer: advances glides, submits settled sweeps
    void onFrameTick();

    /// @brief A digit key: slot 0..9 for keys 1..9, 0; shift stores
    void onDigit(int slot, bool shift);

    /// @brief The audio device's rate and block size
    void setAudioStatus(double sampleRate, int bufferSize);

    /// @param device MIDI input name, empty when none; lastController the last CC received
    void setMidiStatus(const std::string &device, std::optional<int> lastController);

    /// @brief The user muted or unmuted the output
    std::function<void(bool)> onMuteChanged;

    void onSweeps(const SweepResult &result);

    /// @brief A sound grid for map 0 (A) or 1 (B) finished
    void onSoundGrid(std::size_t map, const SoundGridResult &result);

    /// @brief Where MIDI map mode reads and learns assignments; may be null
    void setMidiAssignments(IMidiAssignments *assignments);

    /// @brief MIDI map mode: every control shows its controller tag; click one to learn
    void toggleMidiMapMode();

    bool isInMidiMapMode() const noexcept
    {
        return mMidiMapMode;
    }

    /// @brief Assignments changed (a controller was learned): redraw the tags
    void refreshCcTags();

    /// @brief A controller moved an axis (0 = x, 1 = y) of map 0 (A) or 1 (B) to 0..1
    void setMapAxisFromMidi(size_t map, size_t axis, float value);

    void setLatentFromMidi(size_t index, float value);
    void setXMaxFromMidi(float value);
    void setSClipFromMidi(float value);

    /// @brief The loaded model's analysis; null without a track
    const std::shared_ptr<const TrackAnalysis> &analysis() const noexcept
    {
        return mAnalysis;
    }

    TopBarPresenter &topBar() noexcept
    {
        return mTopBar;
    }

    OutputPresenter &output() noexcept
    {
        return mOutput;
    }

    MapPresenter &mapA() noexcept
    {
        return mMapA;
    }

    MapPresenter &mapB() noexcept
    {
        return mMapB;
    }

    LatentPanelPresenter &latentPanel() noexcept
    {
        return mLatentPanel;
    }

    SnapshotsPresenter &snapshots() noexcept
    {
        return mSnapshots;
    }

private:
    void learn(MidiTarget target);

    LatentState                         &mState;
    IMidiAssignments                    *mAssignments = nullptr;
    bool                                 mMidiMapMode = false;
    SnapshotBank                        &mBank;
    SweepScheduler                       mSweeps;
    std::shared_ptr<const TrackAnalysis> mAnalysis;

    TopBarPresenter      mTopBar;
    OutputPresenter      mOutput;
    MapPresenter         mMapA;
    MapPresenter         mMapB;
    LatentPanelPresenter mLatentPanel;
    SnapshotsPresenter   mSnapshots;
};
