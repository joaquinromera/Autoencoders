/// @file MainComponent.h
/// @brief Main audio component for the autoencoder synthesizer application
/// @author Autoencoder Project
/// @date 2025

#pragma once

#include <Autoencoder.h>
#include <BackgroundDecoder.h>
#include <LatentState.h>
#include <LoadedModel.h>
#include <MainGui.h>
#include <MidiInputHandler.h>
#include <MidiLearn.h>
#include <MidiParameterMapping.h>

#include <JuceHeader.h>

#include <exception>
#include <memory>
#include <string>

//==============================================================================
/// @brief Main audio component for the autoencoder synthesizer
///
/// This component lives inside our window, and this is where you should put all
/// your controls and content. It handles audio processing, MIDI input, and
/// coordinates between the GUI and the autoencoder model.
class MainComponent
    : public juce::AudioAppComponent
    , private LatentState::Listener
    , private juce::Timer
    , private juce::ChangeListener
{
public:
    //==============================================================================
    /// @brief Constructor
    MainComponent();

    /// @brief Destructor
    ~MainComponent() override;

    //==============================================================================
    /// @brief Prepares the audio component for playback
    /// @param samplesPerBlockExpected Expected number of samples per audio block
    /// @param sampleRate The audio sample rate
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /// @brief Processes the next audio block
    /// @param bufferToFill Audio buffer information for the output
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

    /// @brief Releases audio resources when playback stops
    void releaseResources() override;

    //==============================================================================
    /// @brief Paints the component
    /// @param g Graphics context for drawing
    void paint(juce::Graphics &g) override;

    /// @brief Handles component resizing
    void resized() override;

private:
    //==============================================================================
    /// @brief Handles open button clicks to load model files
    void openButtonClicked();

    /// @brief Loads an autoencoder model from a file path
    /// @param path The file path to the model configuration
    void loadAutoencoder(const juce::String &path);

    /// @brief Replaces the playing engine; the old one is destroyed outside the audio lock
    void swapEngine(std::unique_ptr<Autoencoder> &engine);

    /// @brief Starts the worker that measures the piece and sweeps the sliders
    void startBackgroundDecoder(const LoadedModel &model);

    /// @brief Rebuilds the controls for model and starts it at its starting point
    void startModel(const LoadedModel &model);

    /// @brief Hands the engine's last spectrum to the GUI and lets sweeps go out
    void timerCallback() override;

    /// @brief The audio device changed: shows its rate and block size
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    void showAudioStatus();

    // LatentState::Listener: hands the whole state to the engine
    void latentStateChanged(const LatentState &state) override;

    std::unique_ptr<Autoencoder>       mAutoencoder;            ///< The autoencoder model instance
    LatentState                        mLatentState;            ///< Where the user is; outlives the GUI
    MidiParameterMapping               mMidiMapping;            ///< MIDI parameter mapping handler
    MidiLearn                          mMidiLearn;              ///< Binds the next controller moved in MIDI map mode
    MainGui                            mMainGui;                ///< Main GUI component
    MidiInputHandler                   mMidiInputHandler;       ///< MIDI input handler
    std::unique_ptr<BackgroundDecoder> mBackgroundDecoder;      ///< Calls into mMainGui, so destroyed first
    SpectrumFrame                      mSpectrum;               ///< Reused buffer for the engine's last block
    bool                               mSpectrumTick   = false; ///< The spectrum is read every other tick
    bool                               mMuted          = false; ///< Applied to every engine loaded
    int                                mMidiStatusTick = 0;     ///< The MIDI status is refreshed a few times a second

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
