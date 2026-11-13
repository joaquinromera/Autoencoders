/// @file SettingsView.h
/// @brief The settings window: audio device configuration

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

/// @brief JUCE's audio device selector, sized for the settings dialog
///
/// MIDI controller assignments live in the main window's MIDI map mode.
class SettingsView : public juce::Component
{
public:
    explicit SettingsView(juce::AudioDeviceManager &deviceManager);

    void resized() override;

    int getPreferredWidth() const;
    int getPreferredHeight() const;

private:
    juce::AudioDeviceSelectorComponent mAudioDeviceSelector;
};
