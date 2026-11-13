/// @file MidiInputHandler.h
/// @brief MIDI input device management and message handling
/// @author Autoencoder Project
/// @date 2025

#pragma once

#include <MainGui.h>
#include <MidiLearn.h>
#include <MidiParameterMapping.h>

#include <JuceHeader.h>

#include <atomic>

//==============================================================================
/// @brief Handles MIDI input device management and incoming MIDI messages
///
/// This class manages MIDI input devices and routes incoming MIDI control change
/// messages to the appropriate GUI parameters based on the parameter mapping.
class MidiInputHandler
    : private juce::MidiInputCallback
    , private juce::ChangeListener
{
public:
    /// @brief Constructor
    /// @param deviceManager AudioDeviceManager that owns the MIDI I/O
    /// @param gui GUI facade that exposes slider/knob entry points
    /// @param mapping Shared mapping used to resolve CC targets
    /// @param learn Gets each controller first, to bind it when learning
    MidiInputHandler(
        juce::AudioDeviceManager &deviceManager, MainGui &gui, MidiParameterMapping &mapping, MidiLearn &learn
    );

    /// @brief Destructor
    ~MidiInputHandler() override;

    /// @brief Initialize MIDI inputs and set up the default device
    void initialize();

    /// @brief Set the active MIDI input device by index
    /// @param index The index of the MIDI input device to activate
    void setMidiInput(int index);

    /// @brief Message thread: the active input's name, empty when none
    juce::String deviceName() const
    {
        return mSelectedDeviceName;
    }

    /// @brief Any thread: the controller number last received, -1 before any
    int lastController() const noexcept
    {
        return mLastController.load(std::memory_order_relaxed);
    }

private:
    // juce::MidiInputCallback
    /// @brief Handles incoming MIDI messages
    /// @param source The MIDI input source
    /// @param message The received MIDI message
    void handleIncomingMidiMessage(juce::MidiInput *source, const juce::MidiMessage &message) override;

    // juce::ChangeListener (AudioDeviceManager change notifications)
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    // Guard to ensure only one input is enabled at a time.
    void enableOnlyDevice(const juce::String &identifier);

    juce::AudioDeviceManager &mDeviceManager;
    MainGui                  &mGui;
    MidiParameterMapping     &mMapping;
    MidiLearn                &mLearn;
    juce::String              mSelectedDeviceIdentifier;
    juce::String              mSelectedDeviceName;
    std::atomic<int>          mLastController {-1};
    bool                      mIsAddingFromMidiInput  = false;
    bool                      mIsAdjustingMidiDevices = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiInputHandler)
};
