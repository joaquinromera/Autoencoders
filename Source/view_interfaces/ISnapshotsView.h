/// @file ISnapshotsView.h
/// @brief The row of ten snapshot slots and the glide time

#pragma once

#include <IMapView.h>

#include <optional>
#include <vector>

/// @brief What the presenter tells the snapshot row
class ISnapshotsView
{
public:
    virtual ~ISnapshotsView() = default;

    /// @brief The faint outline of the piece every thumbnail is drawn over; once per model
    virtual void setThumbnailOutline(const std::vector<MapPoint> &frames) = 0;

    /// @param slot 0..9
    /// @param filled Whether the slot holds a snapshot
    /// @param at Where the snapshot lies on the thumbnail (map A's axes), if known
    virtual void showSlot(int slot, bool filled, std::optional<MapPoint> at) = 0;

    virtual void showGlideMs(int milliseconds) = 0;
};
