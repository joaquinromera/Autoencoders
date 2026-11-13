/// @file TrackAnalysis.cpp
/// @brief Implementation of TrackAnalysis

#include <TrackAnalysis.h>

#include <stdexcept>

TrackAnalysis::TrackAnalysis(std::shared_ptr<const LatentTrack> track, const std::vector<SliderMinMax> &zRange)
    : mTrack(std::move(track))
    , mProjection(LatentProjection::fromTrack(*mTrack))
{
    if (zRange.size() != mTrack->latentDim())
    {
        throw std::invalid_argument("zRange does not match the track's latent size");
    }
    for (std::size_t dim = 0; dim < zRange.size(); ++dim)
    {
        mHistograms.push_back(
            std::make_unique<LatentHistogram>(mProjection.standardizedCoords(), *mTrack, dim, zRange[dim])
        );
    }
}

const TrackSlice &TrackAnalysis::sliceFor(const MapAxes &axes) const
{
    const SliceKey key {axes.space, axes.x, axes.y};
    if (const auto found = mSlices.find(key); found != mSlices.end())
    {
        return *found->second;
    }

    const auto &coords =
        axes.space == MapAxes::Space::principal ? mProjection.principalCoords() : mProjection.standardizedCoords();
    auto slice = std::make_unique<const TrackSlice>(coords, std::vector<std::size_t> {axes.x, axes.y});
    return *mSlices.emplace(key, std::move(slice)).first->second;
}
