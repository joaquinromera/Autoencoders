/// @file ILatentPanelPresenter.h
/// @brief What the latent panel reports

#pragma once

class ILatentPanelPresenter
{
public:
    virtual ~ILatentPanelPresenter() = default;

    virtual void onSliderDragged(int index, float value) = 0;

    /// @brief A MIDI map mode tag was clicked: index in the order the view got them
    virtual void onCcTagClicked(int index) = 0;
};
