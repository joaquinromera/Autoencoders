/// @file ControlRanges.cpp
/// @brief Implementation of ControlRanges

#include <ControlRanges.h>

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kSClipFloor    = -120.0f;
constexpr float kTopHeadroomDb = 20.0f;
constexpr float kRangeStep     = 10.0f;

float roundUp(float value)
{
    return std::ceil(value / kRangeStep) * kRangeStep;
}

// The top limit wins over the range's minimum, which it can never undercut
// for a valid pair; max/min instead of std::clamp keeps lo > hi defined.
float clampBelow(const SliderMinMax &range, float limit, float value)
{
    return std::max(range.min, std::min({value, range.max, limit}));
}
} // namespace

ControlRanges ControlRanges::forModel(const ModelDescriptor &descriptor)
{
    const auto lowest  = std::min(kSClipFloor, std::floor(descriptor.defaultSClip / kRangeStep) * kRangeStep);
    const auto highest = std::max(0.0f, roundUp(descriptor.defaultSClip));
    const auto top     = roundUp(descriptor.defaultSClip + descriptor.defaultXMax + kTopHeadroomDb);

    // xMax may span the whole plot, so only the plot's edges stop a drag.
    return {
        descriptor.zRange,
        {  0.0f, top - lowest},
        {lowest,      highest},
        top,
    };
}

float ControlRanges::clampZ(std::size_t dim, float value) const
{
    const auto &range = z.at(dim);
    return std::clamp(value, range.min, range.max);
}

float ControlRanges::clampXMax(float value, float sClip) const
{
    return clampBelow(xMax, topMax - sClip, value);
}

float ControlRanges::clampSClip(float value, float xMaxValue) const
{
    return clampBelow(sClip, topMax - xMaxValue, value);
}
