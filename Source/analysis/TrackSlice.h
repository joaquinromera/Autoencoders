/// @file TrackSlice.h
/// @brief Which frames of the piece go with the coordinates a view does not show

#pragma once

#include <CoordTable.h>

#include <cstddef>
#include <span>
#include <vector>

/// @brief Weight of every frame, and their mean
struct SliceWeights
{
    std::vector<double> weights;
    double              mean = 0.0;
};

/// @brief Slices the piece by the coordinates a view leaves out
///
/// A view (a map, a slider's strip) shows some coordinates and keeps them out
/// of the comparison. Every other ("unseen") coordinate decides how much each
/// frame counts: weight = exp(−½ d² / σ²), d the distance to the current
/// coordinates over the unseen ones, σ = sigmaFraction × the piece's RMS spread
/// over them. reference() is what standing on a frame of the piece gives.
class TrackSlice
{
public:
    static constexpr double   kDefaultSigmaFraction = 0.35;
    static constexpr unsigned kDefaultSeed          = 0;

    /// @param coords Frames in the coordinates the view uses; must outlive the slice
    /// @param keep Coordinates the view shows, left out of the comparison
    TrackSlice(
        const CoordTable &coords, const std::vector<std::size_t> &keep, double sigmaFraction = kDefaultSigmaFraction,
        unsigned seed = kDefaultSeed
    );

    /// @brief Weights given the current coordinates c (all dims; kept ones ignored)
    SliceWeights weightsAt(std::span<const double> c) const;

    /// @brief Median mean weight when standing on frames of the piece
    double reference() const noexcept
    {
        return mReference;
    }

    /// @brief mean relative to reference(): 1 is "as on the piece"
    double nearness(double mean) const noexcept
    {
        return mReference > 0.0 ? mean / mReference : 0.0;
    }

    const std::vector<std::size_t> &unseen() const noexcept
    {
        return mUnseen;
    }

private:
    double medianReference(unsigned seed) const;

    const CoordTable        &mCoords;
    std::vector<std::size_t> mUnseen;
    double                   mSigma2    = 0.0;
    double                   mReference = 1.0;
};
