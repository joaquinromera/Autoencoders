/// @file Percentile.h
/// @brief Percentiles with numpy's default (linear) interpolation

#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

/// @brief The p-th percentile (0..100) of values, as numpy.percentile computes it
inline double percentile(std::vector<double> values, double p)
{
    if (values.empty())
    {
        throw std::invalid_argument("percentile of no values");
    }
    std::sort(values.begin(), values.end());

    const double position = (p / 100.0) * static_cast<double>(values.size() - 1);
    const auto   below    = static_cast<std::size_t>(std::floor(position));
    const auto   above    = std::min(below + 1, values.size() - 1);
    return values[below] + ((position - static_cast<double>(below)) * (values[above] - values[below]));
}
