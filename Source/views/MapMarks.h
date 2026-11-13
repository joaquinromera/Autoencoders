/// @file MapMarks.h
/// @brief Drawing the marks and readouts that sit on a map

#pragma once

#include <IMapView.h>

#include <juce_gui_basics/juce_gui_basics.h>

/// @brief Draws the ring, pins, nearness readout and legend of a map
class MapMarks
{
public:
    MapMarks() = delete;

    /// @brief You: a ring with a dot and a faint crosshair; dashed when off the map
    static void ring(juce::Graphics &g, juce::Point<float> at, bool offMap, juce::Rectangle<float> area);

    /// @brief A snapshot slot, kept inside the map
    static void pin(juce::Graphics &g, juce::Point<float> at, int slot, juce::Rectangle<float> area);

    /// @brief "near the piece · 24%" with a gauge
    static void nearness(
        juce::Graphics &g, juce::Rectangle<float> line, const juce::String &text, float gauge, juce::Colour colour
    );

    /// @brief The keys, top down from area's top; the sound layer adds its two scales first
    static void legend(juce::Graphics &g, juce::Rectangle<float> area, MapLayer layer);

    /// @brief Dashed line from where you are to where a glide goes
    static void glideLine(juce::Graphics &g, juce::Point<float> from, juce::Point<float> to);

    /// @brief An axis label written bottom to top along the left edge
    static void verticalLabel(juce::Graphics &g, const juce::String &text, juce::Rectangle<float> area);
};
