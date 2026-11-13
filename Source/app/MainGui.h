/// @file MainGui.h
/// @brief Main GUI component hosting all visual elements
/// @author Autoencoder Project
/// @date 2025

#pragma once

#include <LatentPanelView.h>
#include <MapView.h>
#include <OutputView.h>
#include <SettingsView.h>
#include <SnapshotsView.h>
#include <TopBarView.h>

#include <BackgroundSubmitters.h>
#include <LatentTrack.h>
#include <ModelDescriptor.h>
#include <SnapshotBank.h>
#include <SweepJob.h>

#include <JuceHeader.h>

#include <functional>
#include <memory>
#include <span>

class LatentState;
class MainPresenter;
class IMidiAssignments;

/// @brief Main GUI component hosting all visual elements
///
/// Owns the views and the presenters behind them; every control reads and
/// writes the LatentState it is given.
class MainGui : public juce::Component
{
public:
    MainGui(juce::AudioDeviceManager &deviceManager, LatentState &state);
    ~MainGui() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    /// @brief Digits 1..9, 0 go to a snapshot (shift stores one); M toggles MIDI map mode
    bool keyPressed(const juce::KeyPress &key) override;

    /// @brief Shows a newly loaded model and starts it at its starting point
    void setModel(
        const ModelDescriptor &descriptor, std::shared_ptr<const LatentTrack> track, BackgroundSubmitters submitters
    );

    /// @brief The engine's last block
    void showSpectrum(std::span<const float> bins, float rms);

    /// @brief Called by the GUI timer; lets settled sweep requests go out
    void frameTick();

    void showSweeps(const SweepResult &result);
    void showSoundGrid(std::size_t map, const SoundGridResult &result);

    /// @brief Where MIDI map mode reads and learns controller assignments
    void setMidiAssignments(IMidiAssignments *assignments);

    /// @brief Controller assignments changed: redraw the MIDI tags
    void refreshCcTags();

    void setMapAxisFromMidi(size_t map, size_t axis, double value);

    /// @brief The audio device's rate and block size, for the status and the frequency axis
    void setAudioStatus(double sampleRate, int bufferSize);

    /// @param device MIDI input name, empty when none; lastController -1 before any
    void setMidiStatus(const juce::String &device, int lastController);

    /// @brief The user muted or unmuted the output
    std::function<void(bool)> onMuteChanged;
    void                      setLatentValueFromMidi(size_t index, double value);
    void                      setXMaxFromMidi(double value);
    void                      setSClipFromMidi(double value);

    std::function<void()> onOpenButtonClicked; ///< Callback for open button clicks

private:
    void showSettingsWindow();

    juce::AudioDeviceManager                        &mDeviceManager;
    LatentState                                     &mState;
    juce::Component::SafePointer<juce::DialogWindow> mSettingsWindow;

    SnapshotBank    mSnapshots;
    TopBarView      mTopBar;
    OutputView      mOutput;
    MapView         mMapA;
    MapView         mMapB;
    LatentPanelView mLatentPanel;
    SnapshotsView   mSnapshotsView;

    std::unique_ptr<MainPresenter> mPresenter;
};
