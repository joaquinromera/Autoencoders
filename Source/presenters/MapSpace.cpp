/// @file MapSpace.cpp
/// @brief Implementation of MapSpace

#include <MapSpace.h>

#include <MapDragToLatent.h>
#include <MapLabels.h>

#include <algorithm>
#include <stdexcept>

MapSpace::MapSpace(std::shared_ptr<const TrackAnalysis> analysis, MapAxes axes, const ControlRanges &ranges)
    : mAnalysis(std::move(analysis))
    , mAxes(axes)
    , mRanges(ranges)
{
    if (mAxes.space == MapAxes::Space::principal && mAnalysis == nullptr)
    {
        throw std::invalid_argument("Principal map axes need a track");
    }
    mGeometry = geometryFor();
    mCentre   = centreCoords();
}

std::vector<double> MapSpace::centreCoords() const
{
    std::vector<float> centre;
    for (const auto &range : mRanges.z)
    {
        centre.push_back((range.min + range.max) / 2.0f);
    }
    if (mAnalysis != nullptr)
    {
        centre = mAnalysis->projection().unproject(std::vector<double>(centre.size(), 0.0)); // the mean
    }
    return coordsOf(centre);
}

std::vector<float>
MapSpace::latentWithAxisFromController(std::size_t axis, float value, const std::vector<float> &z) const
{
    const auto   low       = mGeometry.coordsAt({0.0f, 0.0f});
    const auto   high      = mGeometry.coordsAt({1.0f, 1.0f});
    const auto   dim       = axis == 0 ? mAxes.x : mAxes.y;
    const double lo        = axis == 0 ? low.first : low.second;
    const double hi        = axis == 0 ? high.first : high.second;
    const double centre    = std::clamp(mCentre[dim], lo, hi);
    const double t         = std::clamp(static_cast<double>(value), 0.0, 1.0);
    auto         coords    = coordsOf(z);
    coords[dim]            = t < 0.5 ? lo + ((centre - lo) * t / 0.5) : centre + ((hi - centre) * (t - 0.5) / 0.5);
    const auto *projection = mAnalysis != nullptr ? &mAnalysis->projection() : nullptr;
    return MapDragToLatent::apply(z, mAxes, coords[mAxes.x], coords[mAxes.y], projection, mRanges);
}

std::vector<double> MapSpace::coordsOf(const std::vector<float> &z) const
{
    if (mAxes.space == MapAxes::Space::principal)
    {
        return mAnalysis->projection().project(z);
    }
    return {z.begin(), z.end()};
}

std::vector<double> MapSpace::sliceCoordsOf(const std::vector<float> &z) const
{
    const auto &projection = mAnalysis->projection();
    return mAxes.space == MapAxes::Space::principal ? projection.project(z) : projection.standardize(z);
}

MapGeometry::Placement MapSpace::place(const std::vector<double> &coords) const
{
    return mGeometry.place(coords[mAxes.x], coords[mAxes.y]);
}

std::vector<float> MapSpace::latentAt(MapPoint at, const std::vector<float> &z) const
{
    const auto [cx, cy]    = mGeometry.coordsAt(at);
    const auto *projection = mAnalysis != nullptr ? &mAnalysis->projection() : nullptr;
    return MapDragToLatent::apply(z, mAxes, cx, cy, projection, mRanges);
}

std::vector<MapPoint> MapSpace::frameLayout() const
{
    std::vector<MapPoint> points;
    const auto           &track = mAnalysis->track();
    const auto           &pc    = mAnalysis->projection().principalCoords();
    for (std::size_t f = 0; f < track.frames(); ++f)
    {
        const bool principal = mAxes.space == MapAxes::Space::principal;
        points.push_back(mGeometry.positionOf(
            principal ? pc.at(f, mAxes.x) : track.frame(f)[mAxes.x],
            principal ? pc.at(f, mAxes.y) : track.frame(f)[mAxes.y]
        ));
    }
    return points;
}

MapCaption MapSpace::caption() const
{
    return MapLabels::caption(mAxes, mAnalysis != nullptr ? &mAnalysis->projection() : nullptr, place(mCentre).at);
}

MapGeometry MapSpace::geometryFor() const
{
    if (mAxes.space == MapAxes::Space::principal)
    {
        const auto &projection = mAnalysis->projection();
        return {projection.bounds(mAxes.x), projection.bounds(mAxes.y)};
    }
    const auto &x = mRanges.z.at(mAxes.x);
    const auto &y = mRanges.z.at(mAxes.y);
    return {
        {x.min, x.max},
        {y.min, y.max}
    };
}
