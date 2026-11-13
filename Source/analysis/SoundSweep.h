/// @file SoundSweep.h
/// @brief What the decoder makes along one slider's range, the other sliders held

#pragma once

#include <IDecoder.h>
#include <SliderMinMax.h>

#include <cstddef>
#include <vector>

/// @brief Level and tonality at evenly spaced values of one slider, bottom to top
struct SoundProfile
{
    std::vector<double> levels;   ///< dB, see levelDb
    std::vector<double> tonality; ///< 0..1, see tonality
};

/// @brief Where along a slider there is sound, and what kind, given the other sliders
class SoundSweep
{
public:
    static constexpr std::size_t kDefaultSteps = 32;

    SoundSweep() = delete;

    /// @brief Decodes `steps` evenly spaced values of z[dim] over range
    static SoundProfile along(
        const IDecoder &decoder, const std::vector<float> &z, std::size_t dim, SliderMinMax range, float xMax,
        float sClip, std::size_t steps = kDefaultSteps
    );
};
