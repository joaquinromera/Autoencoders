/// @file SoundGrid.cpp
/// @brief Implementation of SoundGrid

#include <SoundGrid.h>

#include <DecodedTonality.h>

SoundGridResult SoundGrid::decode(const IDecoder &decoder, const SoundGridJob &job, LevelRange pieceLevels)
{
    SoundGridResult    result {job.generation, job.size, {}, {}, pieceLevels};
    std::vector<float> pred(decoder.outputDim());
    for (const auto &z : job.points)
    {
        decoder.decodeInto(z, pred);
        result.levels.push_back(levelDb(pred, job.xMax, job.sClip));
        result.tonality.push_back(tonality(pred, job.xMax, job.sClip));
    }
    return result;
}
