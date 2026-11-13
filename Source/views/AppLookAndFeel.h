/// @file AppLookAndFeel.h
/// @brief JUCE's standard widgets in the main window's palette

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/// @brief LookAndFeel_V4 recoloured with Theme: buttons, menus, sliders, text
/// fields and dialogs match the custom-drawn regions
class AppLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AppLookAndFeel();
};
