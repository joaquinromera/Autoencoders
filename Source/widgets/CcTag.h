/// @file CcTag.h
/// @brief The small MIDI tag a control shows in MIDI map mode

#pragma once

#include <CcTagInfo.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

/// @brief "CC 12", "CC –" or "learn…"; click to learn a controller for the control
class CcTag : public juce::Component
{
public:
    static constexpr int kWidth  = 46;
    static constexpr int kHeight = 16;

    std::function<void()> onClicked;

    void setInfo(CcTagInfo info);

    void paint(juce::Graphics &g) override;
    void mouseUp(const juce::MouseEvent &event) override;

private:
    CcTagInfo mInfo;
};
