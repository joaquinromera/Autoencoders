/// @file ITopBarPresenter.h
/// @brief What the top bar reports

#pragma once

class ITopBarPresenter
{
public:
    virtual ~ITopBarPresenter() = default;

    virtual void onOpenModel() = 0;
    virtual void onSettings()  = 0;

    /// @brief The MIDI map button was pressed
    virtual void onMidiMapToggled() = 0;

    /// @brief The Mute button was pressed
    virtual void onMuteToggled() = 0;
};
