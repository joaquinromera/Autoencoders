/// @file CloudDensity.cpp
/// @brief Implementation of CloudDensity

#include <CloudDensity.h>

#include <algorithm>
#include <cmath>

namespace
{
float alpha(double weight, double scale)
{
    return weight < CloudDensity::kWeightFloor ? 0.0f : static_cast<float>(std::min(1.0, weight * scale));
}
} // namespace

double CloudDensity::gain(double mean)
{
    return kBaseGain / std::sqrt(std::max(mean, kMinMean));
}

CloudAlphas CloudDensity::alphas(const SliceWeights &slice)
{
    const double g = gain(slice.mean);
    const auto  &w = slice.weights;

    CloudAlphas result;
    for (std::size_t f = 0; f < w.size(); ++f)
    {
        result.dots.push_back(alpha(w[f], g));
        if (f + 1 < w.size())
        {
            result.segments.push_back(alpha(std::min(w[f], w[f + 1]), kSegmentShare * g));
        }
    }
    return result;
}
