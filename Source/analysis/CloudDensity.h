/// @file CloudDensity.h
/// @brief How strongly to draw each frame of a sliced cloud

#pragma once

#include <TrackSlice.h>

#include <vector>

/// @brief Opacity per frame dot and per segment of the piece's path
struct CloudAlphas
{
    std::vector<float> dots;     ///< One per frame
    std::vector<float> segments; ///< segments[f] joins frame f and f + 1
};

/// @brief Turns slice weights into opacities
///
/// A thin slice is drawn stronger per frame (gain ∝ 1/√mean), so it still
/// reads as a shape; frames under a 0.02 weight are not drawn.
class CloudDensity
{
public:
    static constexpr double kBaseGain     = 0.16;
    static constexpr double kMinMean      = 0.003;
    static constexpr double kWeightFloor  = 0.02;
    static constexpr double kSegmentShare = 0.1;

    CloudDensity() = delete;

    /// @brief Opacity per unit weight for a slice with this mean weight
    static double gain(double mean);

    static CloudAlphas alphas(const SliceWeights &slice);
};
