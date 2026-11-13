/// @file DecodedTonality.cpp
/// @brief Implementation of tonality and tonalStrength

#include <DecodedTonality.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace
{
constexpr double kNoisiest   = 0.15;
constexpr double kMostTonal  = 0.90;
constexpr double kLnPerDbMag = std::numbers::ln10 / 20.0; ///< ln of a magnitude per dB
} // namespace

double tonality(std::span<const float> pred, float xMax, float sClip)
{
    const auto bins = pred.size() > 1 ? pred.subspan(1) : pred;
    if (bins.empty())
    {
        return 0.0;
    }
    // ln(magnitude) is the dB value scaled, so the geometric mean needs no log.
    double sumLn  = 0.0;
    double sumMag = 0.0;
    for (const float bin : bins)
    {
        const double ln  = ((static_cast<double>(bin) * xMax) + sClip) * kLnPerDbMag;
        sumLn           += ln;
        sumMag          += std::exp(ln);
    }
    const auto n = static_cast<double>(bins.size());
    return 1.0 - (std::exp(sumLn / n) / (sumMag / n));
}

double tonalStrength(double tonality)
{
    return std::clamp((tonality - kNoisiest) / (kMostTonal - kNoisiest), 0.0, 1.0);
}
