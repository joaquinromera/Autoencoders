/// @file MapDragToLatent.cpp
/// @brief Implementation of MapDragToLatent

#include <MapDragToLatent.h>

#include <stdexcept>

std::vector<float> MapDragToLatent::apply(
    const std::vector<float> &z, const MapAxes &axes, double cx, double cy, const LatentProjection *projection,
    const ControlRanges &ranges
)
{
    std::vector<float> moved = z;
    if (axes.space == MapAxes::Space::raw)
    {
        moved[axes.x] = static_cast<float>(cx);
        moved[axes.y] = static_cast<float>(cy);
    }
    else
    {
        if (projection == nullptr)
        {
            throw std::invalid_argument("Principal axes need a projection");
        }
        auto c    = projection->project(z);
        c[axes.x] = cx;
        c[axes.y] = cy;
        moved     = projection->unproject(c);
    }

    for (std::size_t i = 0; i < moved.size(); ++i)
    {
        moved[i] = ranges.clampZ(i, moved[i]);
    }
    return moved;
}
