/// @file StartingLatent.cpp
/// @brief Implementation of startingLatent

#include <StartingLatent.h>

#include <cstddef>
#include <limits>

namespace
{
std::vector<float> rangeMidpoints(const ControlRanges &ranges)
{
    std::vector<float> z;
    for (const auto &range : ranges.z)
    {
        z.push_back((range.min + range.max) / 2.0f);
    }
    return z;
}

std::vector<double> trackMean(const LatentTrack &track)
{
    std::vector<double> mean(track.latentDim(), 0.0);
    for (std::size_t f = 0; f < track.frames(); ++f)
    {
        const auto frame = track.frame(f);
        for (std::size_t i = 0; i < mean.size(); ++i)
        {
            mean[i] += frame[i] / static_cast<double>(track.frames());
        }
    }
    return mean;
}

double squaredDistance(std::span<const float> frame, const std::vector<double> &point)
{
    double sum = 0.0;
    for (std::size_t i = 0; i < point.size(); ++i)
    {
        sum += (frame[i] - point[i]) * (frame[i] - point[i]);
    }
    return sum;
}

std::vector<float> frameClosestTo(const LatentTrack &track, const std::vector<double> &point)
{
    std::size_t best         = 0;
    double      bestDistance = std::numeric_limits<double>::infinity();
    for (std::size_t f = 0; f < track.frames(); ++f)
    {
        if (const auto distance = squaredDistance(track.frame(f), point); distance < bestDistance)
        {
            best         = f;
            bestDistance = distance;
        }
    }
    const auto frame = track.frame(best);
    return {frame.begin(), frame.end()};
}
} // namespace

std::vector<float> startingLatent(const ControlRanges &ranges, const LatentTrack *track)
{
    if (track == nullptr || track->frames() == 0 || track->latentDim() != ranges.z.size())
    {
        return rangeMidpoints(ranges);
    }
    return frameClosestTo(*track, trackMean(*track));
}
