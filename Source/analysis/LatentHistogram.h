/// @file LatentHistogram.h
/// @brief One latent dimension's values across the piece, given the other dimensions

#pragma once

#include <CoordTable.h>
#include <LatentTrack.h>
#include <SliderMinMax.h>
#include <TrackSlice.h>

#include <cstddef>
#include <span>
#include <vector>

/// @brief What a slider's histogram strip shows for the current values
struct HistogramView
{
    std::vector<double>
           heights;         ///< One per bin, √(count / max), 0..1, frames at 0 left out; all 0 when under 5 % are off 0
    double zeroShare = 0.0; ///< Share of the counted frames where the dim is exactly 0
    double mass      = 0.0; ///< Slice weight relative to standing on the piece
    bool   empty     = false; ///< No frame of the piece goes with the other values
    bool   flagged   = false; ///< The thumb sits outside the piece's values
};

/// @brief Histogram of dimension dim over its range, counting only the frames
/// whose other dimensions are near the current ones (standardized coordinates)
///
/// A ReLU encoder leaves a dimension at exactly 0 in many frames. Those frames
/// are counted apart, as zeroShare, so they don't flatten every other bin.
class LatentHistogram
{
public:
    static constexpr std::size_t kBins = 44;

    /// @param standardized The track in standardized coordinates; must outlive this
    LatentHistogram(const CoordTable &standardized, const LatentTrack &track, std::size_t dim, SliderMinMax range);

    /// @param s Current z in standardized coordinates
    /// @param thumb Current raw value of dim
    HistogramView at(std::span<const double> s, float thumb) const;

    /// @brief Whether thumb sits where view shows (almost) nothing of the piece
    ///
    /// Lets a slider re-check its flag when only its own value moved, which
    /// does not change its histogram.
    bool isOutside(const HistogramView &view, float thumb) const;

private:
    std::size_t binOf(double value) const;

    TrackSlice               mSlice;
    SliderMinMax             mRange;
    std::vector<std::size_t> mFrameBins; ///< Bin of each frame; kBins when outside the range or at 0
    std::vector<bool>        mAtZero;    ///< Whether each frame has the dim at exactly 0
};
