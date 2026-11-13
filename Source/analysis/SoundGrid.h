/// @file SoundGrid.h
/// @brief Decoding a grid of points over a map: the request, the result and the work

#pragma once

#include <DecodedLevel.h>
#include <IDecoder.h>

#include <cstddef>
#include <cstdint>
#include <vector>

/// @brief Decode every point of a size × size grid
struct SoundGridJob
{
    std::uint64_t                   generation = 0; ///< Increases with every request; only the latest matters
    std::size_t                     size       = 0; ///< Cells per side
    std::vector<std::vector<float>> points;         ///< size × size latent vectors, row by row from the bottom
    float                           xMax  = 0.0f;
    float                           sClip = 0.0f;
};

/// @brief Level and tonality of every grid point, in the job's order
struct SoundGridResult
{
    std::uint64_t       generation = 0;
    std::size_t         size       = 0;
    std::vector<double> levels;      ///< dB, see levelDb
    std::vector<double> tonality;    ///< 0..1, see tonality
    LevelRange          pieceLevels; ///< The piece's levels through the job's xMax and sClip
};

/// @brief Decodes every point of a grid
class SoundGrid
{
public:
    SoundGrid() = delete;

    /// @param pieceLevels The piece's levels at the job's xMax and sClip, passed through
    static SoundGridResult decode(const IDecoder &decoder, const SoundGridJob &job, LevelRange pieceLevels);
};
