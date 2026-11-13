/// @file Theme.h
/// @brief Colours and fonts of the main window, from the UI proposal's palette

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <string>

namespace Theme
{
inline const juce::Colour background {0xff'0b'0c'0e};
inline const juce::Colour bar {0xff'10'12'15};
inline const juce::Colour raised {0xff'16'19'1d};
inline const juce::Colour line {0xff'23'27'2d};
inline const juce::Colour line2 {0xff'2f'34'3c};
inline const juce::Colour text {0xff'e9'ea'ec};
inline const juce::Colour muted {0xff'8b'91'99};
inline const juce::Colour dim {0xff'5a'60'69};
inline const juce::Colour you {0xff'f6'a6'3a};
inline const juce::Colour warn {0xff'e8'70'4a};
inline const juce::Colour ok {0xff'7f'd4'9a};
inline const juce::Colour plot {0xff'0d'0f'12};
inline const juce::Colour grid {0xff'15'18'1c};
inline const juce::Colour axisCross {0xff'1a'1e'23};
inline const juce::Colour cloud {0xff'96'a8'ba};
inline const juce::Colour histogram {0xff'3a'42'50};
inline const juce::Colour sweepNoise {0xff'b4'b8'be}; ///< Sound strip where the decoder makes noise
inline const juce::Colour tonal {0xff'5c'c8'e6};      ///< Sound strip where it makes separate peaks
inline const juce::Colour track {0xff'4a'51'5c};
inline const juce::Colour emptyTrack {0xff'2a'2f'36};
inline const juce::Colour meter {0xff'8f'd3'a4};
inline const juce::Colour bandLine {0xff'cf'd2'd6};

inline juce::Font font(float size, bool bold = false)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

inline juce::Font mono(float size)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain));
}

/// @brief Small uppercase section title, e.g. "OUTPUT"
inline juce::Font title()
{
    return font(10.5f, true).withExtraKerningFactor(0.09f);
}

/// @brief Presenter and string-table texts are UTF-8
inline juce::String utf8(const std::string &value)
{
    return juce::String::fromUTF8(value.c_str());
}

inline juce::String utf8(const char *value)
{
    return juce::String::fromUTF8(value);
}
} // namespace Theme
