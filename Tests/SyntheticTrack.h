#pragma once

#include <LatentTrack.h>

#include <cmath>
#include <random>
#include <vector>

/// A skewed 2-D cloud, rotated by 30°, embedded in 4 dims with two thin noise
/// dims. Its first principal axis is (cos 30°, sin 30°, 0, 0) with the long
/// (exponential) tail towards +.
inline LatentTrack syntheticTrack(std::size_t frames = 4'000, unsigned seed = 1)
{
    std::mt19937                    random(seed);
    std::exponential_distribution<> tail(1.0);
    std::normal_distribution<>      normal(0.0, 1.0);
    const double                    angle = 30.0 * 3.14159265358979 / 180.0;

    std::vector<float> values;
    for (std::size_t f = 0; f < frames; ++f)
    {
        const double u = 3.0 * tail(random);
        const double v = 1.0 * normal(random);
        values.push_back(static_cast<float>((std::cos(angle) * u) - (std::sin(angle) * v)));
        values.push_back(static_cast<float>((std::sin(angle) * u) + (std::cos(angle) * v)));
        values.push_back(static_cast<float>(0.3 * normal(random)));
        values.push_back(static_cast<float>(0.2 * normal(random)));
    }
    return {4, std::move(values)};
}
