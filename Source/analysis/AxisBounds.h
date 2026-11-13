/// @file AxisBounds.h
/// @brief The span of a map axis: where the piece goes, ignoring rare outliers

#pragma once

#include <Percentile.h>

#include <vector>

/// @brief Low and high end of one axis
struct AxisBounds
{
    double lo = 0.0;
    double hi = 1.0;

    /// @brief 0.3rd to 99.7th percentile of values, widened by 8 % on each side
    static AxisBounds robust(const std::vector<double> &values)
    {
        constexpr double kLow = 0.3, kHigh = 99.7, kMargin = 0.08;
        const double     lo     = percentile(values, kLow);
        const double     hi     = percentile(values, kHigh);
        const double     margin = (hi - lo) * kMargin;
        return {lo - margin, hi + margin};
    }
};
