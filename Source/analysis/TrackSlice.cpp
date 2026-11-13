/// @file TrackSlice.cpp
/// @brief Implementation of TrackSlice

#include <TrackSlice.h>

#include <Percentile.h>

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
constexpr std::size_t kReferenceFrames = 300;

std::vector<std::size_t> unseenDims(std::size_t dims, const std::vector<std::size_t> &keep)
{
    std::vector<std::size_t> unseen;
    for (std::size_t k = 0; k < dims; ++k)
    {
        if (std::find(keep.begin(), keep.end(), k) == keep.end())
        {
            unseen.push_back(k);
        }
    }
    return unseen;
}
} // namespace

TrackSlice::TrackSlice(
    const CoordTable &coords, const std::vector<std::size_t> &keep, double sigmaFraction, unsigned seed
)
    : mCoords(coords)
    , mUnseen(unseenDims(coords.dims(), keep))
{
    for (const auto k : mUnseen)
    {
        mSigma2 += coords.meanSquares()[k];
    }
    mSigma2    *= sigmaFraction * sigmaFraction;
    mReference  = medianReference(seed);
}

SliceWeights TrackSlice::weightsAt(std::span<const double> c) const
{
    SliceWeights result {std::vector<double>(mCoords.frames(), 1.0), 1.0};
    if (mSigma2 <= 0.0)
    {
        return result; // nothing unseen, or no spread there: every frame counts
    }

    double sum = 0.0;
    for (std::size_t f = 0; f < mCoords.frames(); ++f)
    {
        double d2 = 0.0;
        for (const auto k : mUnseen)
        {
            const double d  = mCoords.at(f, k) - c[k];
            d2             += d * d;
        }
        result.weights[f]  = std::exp(-0.5 * d2 / mSigma2);
        sum               += result.weights[f];
    }
    result.mean = sum / static_cast<double>(mCoords.frames());
    return result;
}

double TrackSlice::medianReference(unsigned seed) const
{
    std::mt19937                               random(seed);
    std::uniform_int_distribution<std::size_t> pick(0, mCoords.frames() - 1);

    std::vector<double> means;
    for (std::size_t n = 0; n < kReferenceFrames; ++n)
    {
        means.push_back(weightsAt(mCoords.row(pick(random))).mean);
    }
    return percentile(std::move(means), 50.0);
}
