/// @file CcTagInfo.h
/// @brief What a control's MIDI tag shows in MIDI map mode

#pragma once

/// @brief A control's controller, none, or waiting to learn one
struct CcTagInfo
{
    enum class State
    {
        none,
        bound,
        learning
    };

    State state      = State::none;
    int   controller = -1; ///< When bound

    bool operator==(const CcTagInfo &) const = default;
};
