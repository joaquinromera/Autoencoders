/// @file MapSpace.h
/// @brief The coordinate system one map plots

#pragma once

#include <ControlRanges.h>
#include <IMapView.h>
#include <MapAxes.h>
#include <MapGeometry.h>
#include <TrackAnalysis.h>

#include <memory>
#include <vector>

/// @brief Two principal components or two raw dims, and how z maps onto them
///
/// Principal axes need a track analysis; raw axes work without one.
class MapSpace
{
public:
    /// @param analysis Null when the model has no track (raw axes only)
    MapSpace(std::shared_ptr<const TrackAnalysis> analysis, MapAxes axes, const ControlRanges &ranges);

    const MapAxes &axes() const noexcept
    {
        return mAxes;
    }

    bool hasCloud() const noexcept
    {
        return mAnalysis != nullptr;
    }

    /// @brief z in the map's coordinates: principal coordinates or raw z
    std::vector<double> coordsOf(const std::vector<float> &z) const;

    /// @brief z in the coordinates the map's slice compares (needs a cloud)
    std::vector<double> sliceCoordsOf(const std::vector<float> &z) const;

    /// @brief The map's slice of the piece (needs a cloud)
    const TrackSlice &slice() const
    {
        return mAnalysis->sliceFor(mAxes);
    }

    MapGeometry::Placement place(const std::vector<double> &coords) const;

    /// @brief z after dragging to `at` on the map
    std::vector<float> latentAt(MapPoint at, const std::vector<float> &z) const;

    /// @brief z with one axis set from a controller value 0..1
    ///
    /// 0 and 1 are the ends of the axis and 0.5 the piece's mean (the middle
    /// of the range without a track), so a centred stick sits on the piece.
    std::vector<float> latentWithAxisFromController(std::size_t axis, float value, const std::vector<float> &z) const;

    /// @brief Every frame of the piece on the map (needs a cloud)
    std::vector<MapPoint> frameLayout() const;

    MapCaption caption() const;

private:
    MapGeometry         geometryFor() const;
    std::vector<double> centreCoords() const;

    std::shared_ptr<const TrackAnalysis> mAnalysis;
    MapAxes                              mAxes;
    ControlRanges                        mRanges;
    MapGeometry                          mGeometry;
    std::vector<double>                  mCentre; ///< The piece's mean in map coordinates
};
