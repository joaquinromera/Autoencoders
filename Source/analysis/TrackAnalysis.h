/// @file TrackAnalysis.h
/// @brief Everything the views derive from a model's latent track, built once per model

#pragma once

#include <LatentHistogram.h>
#include <LatentProjection.h>
#include <LatentTrack.h>
#include <MapAxes.h>
#include <SliderMinMax.h>
#include <TrackSlice.h>

#include <map>
#include <memory>
#include <tuple>
#include <vector>

/// @brief Projection, map slices and slider histograms of one track
///
/// Immutable once built apart from the slice cache; message thread only.
/// Not copyable: slices and histograms point into the projection's tables.
class TrackAnalysis
{
public:
    TrackAnalysis(std::shared_ptr<const LatentTrack> track, const std::vector<SliderMinMax> &zRange);

    TrackAnalysis(const TrackAnalysis &)            = delete;
    TrackAnalysis &operator=(const TrackAnalysis &) = delete;

    const LatentTrack &track() const noexcept
    {
        return *mTrack;
    }

    const LatentProjection &projection() const noexcept
    {
        return mProjection;
    }

    /// @brief The slice for a map: principal pairs slice principal
    /// coordinates, raw pairs slice standardized ones. Built on first use.
    const TrackSlice &sliceFor(const MapAxes &axes) const;

    /// @brief The histogram strip of latent dimension dim
    const LatentHistogram &histogram(std::size_t dim) const
    {
        return *mHistograms.at(dim);
    }

private:
    using SliceKey = std::tuple<MapAxes::Space, std::size_t, std::size_t>;

    std::shared_ptr<const LatentTrack>                            mTrack;
    LatentProjection                                              mProjection;
    std::vector<std::unique_ptr<LatentHistogram>>                 mHistograms;
    mutable std::map<SliceKey, std::unique_ptr<const TrackSlice>> mSlices;
};
