/// @file SweepJob.h
/// @brief A request for, and the result of, decoding along every slider

#pragma once

#include <DecodedLevel.h>
#include <SoundSweep.h>

#include <cstdint>
#include <vector>

/// @brief Sweep every slider's range from z, the others held
struct SweepJob
{
    std::uint64_t      generation = 0; ///< Increases with every request; only the latest matters
    std::vector<float> z;
    float              xMax  = 0.0f;
    float              sClip = 0.0f;
};

/// @brief What the decoder makes along each slider, one profile per latent dimension
struct SweepResult
{
    std::uint64_t             generation = 0;
    std::vector<SoundProfile> profiles;
    LevelRange                pieceLevels; ///< The piece's levels through the job's xMax and sClip
};
