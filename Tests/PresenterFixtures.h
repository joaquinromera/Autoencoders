#pragma once

#include <ControlRanges.h>
#include <LatentState.h>
#include <SyntheticTrack.h>
#include <TrackAnalysis.h>

#include <memory>

/// The synthetic 4-D track, its analysis and ranges wide enough for it.
struct SyntheticModel
{
    std::shared_ptr<const LatentTrack> track = std::make_shared<const LatentTrack>(syntheticTrack(2'000));
    ControlRanges                      ranges {
        {{-5.0f, 25.0f}, {-5.0f, 25.0f}, {-2.0f, 2.0f}, {-2.0f, 2.0f}},
        {0.0f, 150.0f},
        {-120.0f, 0.0f}
    };
    std::shared_ptr<const TrackAnalysis> analysis = std::make_shared<const TrackAnalysis>(track, ranges.z);

    std::vector<float> frame(std::size_t f) const
    {
        const auto values = track->frame(f);
        return {values.begin(), values.end()};
    }
};
