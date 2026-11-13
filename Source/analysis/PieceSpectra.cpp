/// @file PieceSpectra.cpp
/// @brief Implementation of PieceSpectra

#include <PieceSpectra.h>

#include <Percentile.h>

#include <random>

PieceSpectra::PieceSpectra(std::vector<std::vector<float>> frames)
    : mFrames(std::move(frames))
{
}

PieceSpectra PieceSpectra::decode(const IDecoder &decoder, const LatentTrack &track, std::size_t samples, unsigned seed)
{
    std::mt19937                               random(seed);
    std::uniform_int_distribution<std::size_t> pick(0, track.frames() - 1);

    std::vector<std::vector<float>> frames;
    std::vector<float>              z(track.latentDim());
    for (std::size_t n = 0; n < samples; ++n)
    {
        const auto frame = track.frame(pick(random));
        z.assign(frame.begin(), frame.end());
        auto &pred = frames.emplace_back(decoder.outputDim());
        decoder.decodeInto(z, pred);
    }
    return PieceSpectra(std::move(frames));
}

LevelRange PieceSpectra::levels(float xMax, float sClip) const
{
    std::vector<double> levels;
    levels.reserve(mFrames.size());
    for (const auto &pred : mFrames)
    {
        levels.push_back(levelDb(pred, xMax, sClip));
    }
    return {percentile(levels, 2.0), percentile(levels, 98.0)};
}
