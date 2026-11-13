/// @file SoundMapLayer.h
/// @brief The sound view of one map: what the decoder makes at every point of it

#pragma once

#include <IMapView.h>
#include <JobScheduler.h>
#include <MapSpace.h>
#include <SoundGrid.h>

#include <cstddef>
#include <vector>

/// @brief Asks for a grid of decodes over a map and shows their level and tonality
///
/// Each cell is decoded at the z a drag to the cell's centre would give, so
/// the colour under a point is what the ring would sound like there.
class SoundMapLayer
{
public:
    static constexpr std::size_t kCells = 24; ///< Cells per side

    using Scheduler = JobScheduler<SoundGridJob>;

    SoundMapLayer(IMapView &view, Scheduler::Clock clock);

    /// @brief Where grid jobs go; none means requests are dropped
    void setSubmitter(Scheduler::Submitter submitter);

    /// @brief Asks for the grid of space around z, submitted once requests settle
    void request(const MapSpace &space, const std::vector<float> &z, float xMax, float sClip);

    /// @brief The map's axes changed: clears what is shown and drops what is on its way
    void reset();

    /// @brief Submits a settled request; called by the GUI timer
    void tick();

    /// @brief A grid finished; shown unless it is stale
    void onResult(const SoundGridResult &result);

    /// @brief The latent vectors of a size × size grid over space, row by row from the bottom
    static std::vector<std::vector<float>>
    gridPoints(const MapSpace &space, const std::vector<float> &z, std::size_t size);

private:
    IMapView &mView;
    Scheduler mScheduler;
};
