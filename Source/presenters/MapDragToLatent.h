/// @file MapDragToLatent.h
/// @brief The latent vector a drag on a map leads to

#pragma once

#include <ControlRanges.h>
#include <LatentProjection.h>
#include <MapAxes.h>

#include <vector>

/// @brief Turns a map position into a whole latent vector
class MapDragToLatent
{
public:
    MapDragToLatent() = delete;

    /// @brief z with the map's two coordinates set to cx, cy, clamped to the ranges
    ///
    /// Principal axes: c = project(z), set c[x] and c[y], z′ = unproject(c),
    /// so the other principal coordinates stay. Raw axes: set z[x] and z[y].
    /// @param projection Required for principal axes
    static std::vector<float> apply(
        const std::vector<float> &z, const MapAxes &axes, double cx, double cy, const LatentProjection *projection,
        const ControlRanges &ranges
    );
};
