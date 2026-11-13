/// @file ITopBarView.h
/// @brief The strip at the top: which model is loaded

#pragma once

#include <string>

/// @brief What the presenter tells the top bar
class ITopBarView
{
public:
    virtual ~ITopBarView() = default;

    /// @param name Model name shown on the model button
    /// @param details e.g. "8 latent dims · frugally-deep · trained on piece.wav (2:00)"
    virtual void showModel(const std::string &name, const std::string &details) = 0;

    /// @brief Nothing is loaded yet
    virtual void showNoModel() = 0;

    /// @brief Whether MIDI map mode is on (the MIDI map button lit)
    virtual void showMidiMapMode(bool on) = 0;

    /// @brief e.g. "IAC Driver Bus 1 · CC 21"
    virtual void showMidiStatus(const std::string &text) = 0;

    /// @brief e.g. "48 kHz · 512"
    virtual void showOutputStatus(const std::string &text) = 0;

    /// @brief e.g. "model trained at 22.05 kHz"; empty hides the warning
    virtual void showRateWarning(const std::string &text) = 0;

    virtual void showMuted(bool muted) = 0;
};
