/// @file SoundSweep.cpp
/// @brief Implementation of SoundSweep

#include <SoundSweep.h>

#include <DecodedLevel.h>
#include <DecodedTonality.h>

SoundProfile SoundSweep::along(
    const IDecoder &decoder, const std::vector<float> &z, std::size_t dim, SliderMinMax range, float xMax, float sClip,
    std::size_t steps
)
{
    SoundProfile       profile;
    std::vector<float> point = z;
    std::vector<float> pred(decoder.outputDim());
    for (std::size_t step = 0; step < steps; ++step)
    {
        const auto t = steps > 1 ? static_cast<float>(step) / static_cast<float>(steps - 1) : 0.0f;
        point[dim]   = range.min + (t * (range.max - range.min));
        decoder.decodeInto(point, pred);
        profile.levels.push_back(levelDb(pred, xMax, sClip));
        profile.tonality.push_back(tonality(pred, xMax, sClip));
    }
    return profile;
}
