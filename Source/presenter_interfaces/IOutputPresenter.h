/// @file IOutputPresenter.h
/// @brief What the output region reports

#pragma once

/// @brief The part of the Floor/Range band being dragged
enum class BandPart
{
    band,    ///< The space between the lines: moves both
    ceiling, ///< The top line: Range only
    floor    ///< The bottom line: Floor only, the top stays
};

class IOutputPresenter
{
public:
    virtual ~IOutputPresenter() = default;

    /// @brief The plot was pressed at a level
    /// @param reachDb How close to a line still counts as on it
    virtual void onBandPressed(float db, float reachDb) = 0;

    /// @brief The press moved; the level is where the cursor is now
    virtual void onBandDragged(float db)      = 0;
    virtual void onFloorEdited(float floorDb) = 0;
    virtual void onRangeEdited(float rangeDb) = 0;

    /// @brief A MIDI map mode tag was clicked: index in the order the view got them
    virtual void onCcTagClicked(int index) = 0;
};
