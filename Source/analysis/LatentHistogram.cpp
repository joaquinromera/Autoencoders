/// @file LatentHistogram.cpp
/// @brief Implementation of LatentHistogram

#include <LatentHistogram.h>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace
{
constexpr double kEmptyBelowMass    = 0.03;
constexpr double kFlaggedBelowShare = 0.05;
// Fewer frames than this off 0 are slice weights of far frames, not a shape
constexpr double kBarsBelowShare = 0.05;
} // namespace

LatentHistogram::LatentHistogram(
    const CoordTable &standardized, const LatentTrack &track, std::size_t dim, SliderMinMax range
)
    : mSlice(standardized, {dim})
    , mRange(range)
{
    for (std::size_t f = 0; f < track.frames(); ++f)
    {
        const double value = track.frame(f)[dim];
        const double bin   = std::floor((value - range.min) / (range.max - range.min) * kBins);
        const bool   zero  = value == 0.0;
        mAtZero.push_back(zero);
        mFrameBins.push_back(!zero && bin >= 0.0 && bin < kBins ? static_cast<std::size_t>(bin) : kBins);
    }
}

HistogramView LatentHistogram::at(std::span<const double> s, float thumb) const
{
    const auto          slice = mSlice.weightsAt(s);
    std::vector<double> counts(kBins, 0.0);
    double              atZero = 0.0;
    for (std::size_t f = 0; f < mFrameBins.size(); ++f)
    {
        if (mFrameBins[f] < kBins)
        {
            counts[mFrameBins[f]] += slice.weights[f];
        }
        atZero += mAtZero[f] ? slice.weights[f] : 0.0;
    }

    const double  max   = *std::max_element(counts.begin(), counts.end());
    const double  total = std::accumulate(counts.begin(), counts.end(), atZero);
    HistogramView view {
        std::vector<double>(kBins, 0.0),
        total > 0.0 ? atZero / total : 0.0,
        mSlice.nearness(slice.mean)
    };
    if (1.0 - view.zeroShare >= kBarsBelowShare)
    {
        std::transform(counts.begin(), counts.end(), view.heights.begin(), [max](double c) {
            return max > 0.0 ? std::sqrt(c / max) : 0.0;
        });
    }
    view.empty   = view.mass < kEmptyBelowMass;
    view.flagged = isOutside(view, thumb);
    return view;
}

bool LatentHistogram::isOutside(const HistogramView &view, float thumb) const
{
    if (view.empty)
    {
        return false;
    }
    const auto bin   = binOf(thumb);
    const auto first = bin > 0 ? bin - 1 : bin;
    const auto last  = std::min(bin + 1, kBins - 1);
    if (view.zeroShare >= kFlaggedBelowShare && first <= binOf(0.0) && binOf(0.0) <= last)
    {
        return false; // the frames resting at 0 are there, though not in the bins
    }
    // Heights are √(count / max): squared, they are each bin's share of the tallest.
    double share = 0.0;
    for (auto b = first; b <= last; ++b)
    {
        share = std::max(share, view.heights[b] * view.heights[b]);
    }
    return share < kFlaggedBelowShare;
}

std::size_t LatentHistogram::binOf(double value) const
{
    const double bin = std::floor((value - mRange.min) / (mRange.max - mRange.min) * kBins);
    return static_cast<std::size_t>(std::clamp(bin, 0.0, static_cast<double>(kBins - 1)));
}
