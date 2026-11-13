/// @file LogSpectrumResampler.h
/// @brief Linear FFT bins to points evenly spaced on a log frequency axis

#pragma once

#include <cstddef>
#include <span>
#include <vector>

/// @brief Resamples a spectrum to `points` log-spaced frequencies, lowHz..sr/2
class LogSpectrumResampler
{
public:
    static constexpr std::size_t kDefaultPoints = 150;
    static constexpr double      kDefaultLowHz  = 55.0;

    /// @param binCount Bins of the spectrum, DC to Nyquist inclusive
    LogSpectrumResampler(
        std::size_t binCount, double sampleRate, std::size_t points = kDefaultPoints, double lowHz = kDefaultLowHz
    );

    const std::vector<double> &frequencies() const noexcept
    {
        return mFrequencies;
    }

    /// @brief Linearly interpolated value of bins at each frequency
    void resampleInto(std::span<const float> bins, std::vector<float> &out) const;

private:
    std::vector<double> mFrequencies;
    std::vector<double> mBinPositions; ///< Fractional bin index of each frequency
};
