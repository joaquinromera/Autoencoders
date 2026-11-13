/// @file BandDrag.h
/// @brief What dragging the Floor/Range band does to xMax and sClip

#pragma once

#include <ControlRanges.h>
#include <IOutputPresenter.h>

#include <algorithm>

/// @brief xMax (Range) and sClip (Floor)
struct BandValues
{
    float xMax  = 0.0f;
    float sClip = 0.0f;
};

/// @brief Applies a drag of the band in the output region
///
/// The band moves the floor and keeps the range (level); the top line
/// changes the range (contrast); the bottom line moves the floor and keeps
/// the top line where it is. The drag stops at the first limit it meets, so
/// a part never pulls the other line along.
class BandDrag
{
public:
    BandDrag() = delete;

    /// @param deltaDb How far the part moved since the drag started, up positive
    /// @param start xMax and sClip when the drag started
    static BandValues apply(BandPart part, float deltaDb, BandValues start, const ControlRanges &ranges)
    {
        switch (part)
        {
            case BandPart::band:
                return moveBand(deltaDb, start, ranges);
            case BandPart::ceiling:
                return moveTop(deltaDb, start, ranges);
            case BandPart::floor:
                return moveFloor(deltaDb, start, ranges);
        }
        return start;
    }

private:
    static BandValues moveBand(float deltaDb, BandValues start, const ControlRanges &ranges)
    {
        const float headroom = ranges.topMax - (start.sClip + start.xMax);
        const float highest  = std::min(ranges.sClip.max - start.sClip, headroom);
        const float d        = limit(deltaDb, ranges.sClip.min - start.sClip, highest);
        return {start.xMax, start.sClip + d};
    }

    static BandValues moveTop(float deltaDb, BandValues start, const ControlRanges &ranges)
    {
        const float headroom = ranges.topMax - (start.sClip + start.xMax);
        const float highest  = std::min(ranges.xMax.max - start.xMax, headroom);
        const float d        = limit(deltaDb, ranges.xMax.min - start.xMax, highest);
        return {start.xMax + d, start.sClip};
    }

    // Raising the floor shrinks the range by as much, so both ranges bound the move.
    static BandValues moveFloor(float deltaDb, BandValues start, const ControlRanges &ranges)
    {
        const float lowest  = std::max(ranges.sClip.min - start.sClip, start.xMax - ranges.xMax.max);
        const float highest = std::min(ranges.sClip.max - start.sClip, start.xMax - ranges.xMax.min);
        const float d       = limit(deltaDb, lowest, highest);
        return {start.xMax - d, start.sClip + d};
    }

    static float limit(float value, float lowest, float highest)
    {
        return std::max(lowest, std::min(value, highest));
    }
};
