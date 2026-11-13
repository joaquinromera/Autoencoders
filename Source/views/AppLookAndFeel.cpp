/// @file AppLookAndFeel.cpp
/// @brief Implementation of AppLookAndFeel

#include <AppLookAndFeel.h>

#include <Theme.h>

AppLookAndFeel::AppLookAndFeel()
{
    using LF = juce::LookAndFeel_V4;
    setColourScheme(
        LF::ColourScheme {
            Theme::background,
            Theme::bar,
            Theme::raised,
            Theme::line2,
            Theme::text,
            Theme::raised,
            Theme::text,
            Theme::you,
            Theme::text
        }
    );

    setColour(juce::ResizableWindow::backgroundColourId, Theme::background);
    setColour(juce::TextButton::buttonColourId, Theme::raised);
    setColour(juce::TextButton::textColourOffId, Theme::text);
    setColour(juce::ComboBox::outlineColourId, Theme::line2);
    setColour(juce::PopupMenu::backgroundColourId, Theme::raised);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Theme::line2);
    setColour(juce::PopupMenu::headerTextColourId, Theme::muted);
    setColour(juce::TextEditor::backgroundColourId, Theme::plot);
    setColour(juce::TextEditor::outlineColourId, Theme::line);
    setColour(juce::TextEditor::focusedOutlineColourId, Theme::you);
    setColour(juce::TextEditor::highlightColourId, Theme::you.withAlpha(0.3f));
    setColour(juce::CaretComponent::caretColourId, Theme::you);
    setColour(juce::Slider::textBoxTextColourId, Theme::text);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::AlertWindow::backgroundColourId, Theme::raised);
    setColour(juce::AlertWindow::textColourId, Theme::text);
}
