/// @file MidiTarget.h
/// @brief A control a MIDI controller can drive

#pragma once

#include <cstddef>

/// @brief One latent dimension, xMax, sClip, or one axis of a map
struct MidiTarget
{
    enum class Kind
    {
        latent,
        xMax,
        sClip,
        mapAxis
    };

    Kind        kind  = Kind::latent;
    std::size_t index = 0; ///< latent: the dim; mapAxis: map × 2 + axis (0 = x, 1 = y)

    static MidiTarget latentDim(std::size_t dim)
    {
        return {Kind::latent, dim};
    }

    static MidiTarget xMaxControl()
    {
        return {Kind::xMax, 0};
    }

    static MidiTarget sClipControl()
    {
        return {Kind::sClip, 0};
    }

    static MidiTarget mapAxis(std::size_t map, std::size_t axis)
    {
        return {Kind::mapAxis, (map * 2) + axis};
    }

    bool operator==(const MidiTarget &) const = default;
};
