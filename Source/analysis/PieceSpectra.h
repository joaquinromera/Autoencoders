/// @file PieceSpectra.h
/// @brief Decoded spectra of a sample of the piece, to measure its levels at any floor and range

#pragma once

#include <DecodedLevel.h>
#include <IDecoder.h>
#include <LatentTrack.h>

#include <cstddef>
#include <vector>

/// @brief The decoder's output for seeded frames of the piece, kept to scale the sound strips
///
/// The levels depend on xMax and sClip, which the user changes; keeping the
/// decoded frames lets them be measured again without decoding.
class PieceSpectra
{
public:
    static constexpr std::size_t kDefaultSamples = 600;

    /// @brief Decodes `samples` frames picked with a seeded generator
    static PieceSpectra
    decode(const IDecoder &decoder, const LatentTrack &track, std::size_t samples = kDefaultSamples, unsigned seed = 0);

    /// @brief 2nd to 98th percentile of the frames' levels through xMax and sClip
    LevelRange levels(float xMax, float sClip) const;

private:
    explicit PieceSpectra(std::vector<std::vector<float>> frames);

    std::vector<std::vector<float>> mFrames;
};
