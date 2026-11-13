/// @file DecodedLevel.cpp
/// @brief Implementation of levelDb and brightness

#include <DecodedLevel.h>

#include <algorithm>
#include <cmath>

double levelDb(std::span<const float> pred, float xMax, float sClip)
{
    double power = 0.0;
    for (const float bin : pred)
    {
        power += std::pow(10.0, ((static_cast<double>(bin) * xMax) + sClip) / 10.0);
    }
    return 10.0 * std::log10(power);
}

double brightness(double level, const LevelRange &range)
{
    constexpr double kDarkBelowLowDb = 15.0;
    const double     floor           = range.lo - kDarkBelowLowDb;
    return std::clamp((level - floor) / (range.hi - floor), 0.0, 1.0);
}
