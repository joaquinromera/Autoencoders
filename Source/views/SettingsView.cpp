/// @file SettingsView.cpp
/// @brief Implementation of the SettingsView class

#include <SettingsView.h>

namespace
{
constexpr int kPadding = 5;
constexpr int kWidth   = 520;
constexpr int kHeight  = 360;
} // namespace

SettingsView::SettingsView(juce::AudioDeviceManager &deviceManager)
    : mAudioDeviceSelector(deviceManager, 2, 2, 2, 2, true, false, false, false)
{
    addAndMakeVisible(mAudioDeviceSelector);
    setSize(getPreferredWidth(), getPreferredHeight());
}

void SettingsView::resized()
{
    mAudioDeviceSelector.setBounds(getLocalBounds().reduced(kPadding));
}

int SettingsView::getPreferredWidth() const
{
    return kWidth;
}

int SettingsView::getPreferredHeight() const
{
    return kHeight;
}
