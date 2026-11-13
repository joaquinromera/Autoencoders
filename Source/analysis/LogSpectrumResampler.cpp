/// @file LogSpectrumResampler.cpp
/// @brief Implementation of LogSpectrumResampler

#include <LogSpectrumResampler.h>

#include <algorithm>
#include <cmath>

LogSpectrumResampler::LogSpectrumResampler(std::size_t binCount, double sampleRate, std::size_t points, double lowHz)
{
    const double nyquist = sampleRate / 2.0;
    for (std::size_t p = 0; p < points; ++p)
    {
        const double t         = points > 1 ? static_cast<double>(p) / static_cast<double>(points - 1) : 0.0;
        const double frequency = lowHz * std::pow(nyquist / lowHz, t);
        mFrequencies.push_back(frequency);
        mBinPositions.push_back(frequency / nyquist * static_cast<double>(binCount - 1));
    }
}

void LogSpectrumResampler::resampleInto(std::span<const float> bins, std::vector<float> &out) const
{
    out.resize(mBinPositions.size());
    const auto last = bins.size() - 1;
    for (std::size_t p = 0; p < mBinPositions.size(); ++p)
    {
        const double position = std::clamp(mBinPositions[p], 0.0, static_cast<double>(last));
        const auto   below    = static_cast<std::size_t>(position);
        const auto   above    = std::min(below + 1, last);
        const auto   t        = static_cast<float>(position - static_cast<double>(below));
        out[p]                = bins[below] + (t * (bins[above] - bins[below]));
    }
}
