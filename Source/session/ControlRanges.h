/// @file ControlRanges.h
/// @brief The ranges every control of a loaded model can move in

#pragma once

#include <ModelDescriptor.h>
#include <SliderMinMax.h>

#include <cstddef>
#include <vector>

/// @brief Ranges of the latent dimensions, xMax and sClip
///
/// sClip is the floor of the output in dB and sClip + xMax its top. The top
/// has a limit of its own, so the output plot can show every reachable band.
struct ControlRanges
{
    std::vector<SliderMinMax> z; ///< One range per latent dimension
    SliderMinMax              xMax {0.0f, 180.0f};
    SliderMinMax              sClip {-120.0f, 0.0f};
    float                     topMax = 60.0f; ///< Highest sClip + xMax

    /// @brief zRange from the file; sClip from −120 to 0 and the top up to
    /// 20 dB over the default top, in steps of 10 that cover the defaults;
    /// xMax up to the distance from the lowest sClip to the highest top
    static ControlRanges forModel(const ModelDescriptor &descriptor);

    float clampZ(std::size_t dim, float value) const;

    /// @brief Clamps xMax to its range and keeps sClip + xMax under topMax
    float clampXMax(float value, float sClip) const;

    /// @brief Clamps sClip to its range and keeps sClip + xMax under topMax
    float clampSClip(float value, float xMaxValue) const;
};
