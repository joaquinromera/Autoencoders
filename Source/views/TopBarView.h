/// @file TopBarView.h
/// @brief The strip at the top of the window

#pragma once

#include <ITopBarPresenter.h>
#include <ITopBarView.h>

#include <juce_gui_basics/juce_gui_basics.h>

/// @brief Model button and details on the left; MIDI and output status, the
/// rate warning, MIDI map, Mute and settings on the right
class TopBarView
    : public juce::Component
    , public ITopBarView
{
public:
    TopBarView();

    void setPresenter(ITopBarPresenter *presenter)
    {
        mPresenter = presenter;
    }

    void showModel(const std::string &name, const std::string &details) override;
    void showNoModel() override;
    void showMidiMapMode(bool on) override;
    void showMidiStatus(const std::string &text) override;
    void showOutputStatus(const std::string &text) override;
    void showRateWarning(const std::string &text) override;
    void showMuted(bool muted) override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    void setUpButton(juce::TextButton &button, std::function<void(ITopBarPresenter &)> onClick);
    void paintStatus(juce::Graphics &g, juce::Rectangle<int> area, const char *label, const juce::String &value) const;

    ITopBarPresenter    *mPresenter = nullptr;
    juce::TextButton     mModelButton;
    juce::Label          mDetails;
    juce::TextButton     mMidiMapButton;
    juce::TextButton     mMuteButton;
    juce::TextButton     mSettingsButton;
    juce::String         mMidiStatus;
    juce::String         mOutputStatus;
    juce::String         mRateWarning;
    juce::Rectangle<int> mMidiArea, mOutputArea, mWarningArea;
};
