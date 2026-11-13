/// @file MapGeometry.h
/// @brief Between a map's coordinates and its 0..1 drawing space

#pragma once

#include <AxisBounds.h>
#include <IMapView.h>

#include <algorithm>
#include <utility>

/// @brief Maps two coordinates within their bounds to 0..1, and back
class MapGeometry
{
public:
    /// @brief A point on the map and whether it had to be pulled in to the edge
    struct Placement
    {
        MapPoint at;
        bool     offMap = false;
    };

    MapGeometry() = default;

    MapGeometry(AxisBounds x, AxisBounds y)
        : mX(x)
        , mY(y)
    {
    }

    Placement place(double cx, double cy) const
    {
        const auto at     = positionOf(cx, cy);
        const bool offMap = at.x < 0.0f || at.x > 1.0f || at.y < 0.0f || at.y > 1.0f;
        return {{std::clamp(at.x, 0.0f, 1.0f), std::clamp(at.y, 0.0f, 1.0f)}, offMap};
    }

    /// @brief Unclamped position, for frames that may lie outside the bounds
    MapPoint positionOf(double cx, double cy) const
    {
        return {static_cast<float>((cx - mX.lo) / (mX.hi - mX.lo)), static_cast<float>((cy - mY.lo) / (mY.hi - mY.lo))};
    }

    std::pair<double, double> coordsAt(MapPoint at) const
    {
        return {mX.lo + (at.x * (mX.hi - mX.lo)), mY.lo + (at.y * (mY.hi - mY.lo))};
    }

private:
    AxisBounds mX;
    AxisBounds mY;
};
