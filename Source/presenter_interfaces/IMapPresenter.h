/// @file IMapPresenter.h
/// @brief What a map reports

#pragma once

#include <IMapView.h>

class IMapPresenter
{
public:
    virtual ~IMapPresenter() = default;

    /// @brief The user dragged to x, y (0..1, left to right, bottom to top)
    virtual void onMapDragged(float x, float y) = 0;

    /// @brief An entry of the Projection menu was chosen
    virtual void onProjectionSelected(int id) = 0;

    /// @brief The user picked what the map's background shows
    virtual void onLayerSelected(MapLayer layer) = 0;

    /// @brief A MIDI map mode tag was clicked: index in the order the view got them
    virtual void onCcTagClicked(int index) = 0;
};
