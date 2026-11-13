/// @file SoundMapLayer.cpp
/// @brief Implementation of SoundMapLayer

#include <SoundMapLayer.h>

#include <DecodedTonality.h>

SoundMapLayer::SoundMapLayer(IMapView &view, Scheduler::Clock clock)
    : mView(view)
    , mScheduler(std::move(clock))
{
}

void SoundMapLayer::setSubmitter(Scheduler::Submitter submitter)
{
    mScheduler.setSubmitter(std::move(submitter));
}

void SoundMapLayer::request(const MapSpace &space, const std::vector<float> &z, float xMax, float sClip)
{
    mScheduler.request({0, kCells, gridPoints(space, z, kCells), xMax, sClip});
}

void SoundMapLayer::reset()
{
    mScheduler.invalidate();
    mView.showSoundGrid(0, {}, {});
}

void SoundMapLayer::tick()
{
    mScheduler.tick();
}

void SoundMapLayer::onResult(const SoundGridResult &result)
{
    if (!mScheduler.isLatest(result.generation))
    {
        return;
    }
    std::vector<float> lit;
    std::vector<float> tonal;
    for (std::size_t k = 0; k < result.levels.size(); ++k)
    {
        lit.push_back(static_cast<float>(brightness(result.levels[k], result.pieceLevels)));
        tonal.push_back(static_cast<float>(tonalStrength(result.tonality[k])));
    }
    mView.showSoundGrid(result.size, lit, tonal);
}

std::vector<std::vector<float>>
SoundMapLayer::gridPoints(const MapSpace &space, const std::vector<float> &z, std::size_t size)
{
    std::vector<std::vector<float>> points;
    const auto                      cell = 1.0f / static_cast<float>(size);
    for (std::size_t row = 0; row < size; ++row)
    {
        for (std::size_t column = 0; column < size; ++column)
        {
            const MapPoint centre {(static_cast<float>(column) + 0.5f) * cell, (static_cast<float>(row) + 0.5f) * cell};
            points.push_back(space.latentAt(centre, z));
        }
    }
    return points;
}
