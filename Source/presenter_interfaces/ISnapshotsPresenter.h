/// @file ISnapshotsPresenter.h
/// @brief What the snapshot row reports

#pragma once

class ISnapshotsPresenter
{
public:
    virtual ~ISnapshotsPresenter() = default;

    /// @brief A click: glide to the snapshot
    virtual void onSlotTapped(int slot) = 0;

    /// @brief Held (or shift-clicked): store where you are
    virtual void onSlotHeld(int slot) = 0;

    virtual void onGlideChanged(int milliseconds) = 0;
};
