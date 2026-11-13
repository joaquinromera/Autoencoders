/// @file GlideRamp.h
/// @brief Moves the latent vector from where it is to a snapshot over time

#pragma once

#include <optional>
#include <vector>

/// @brief A linear glide between two latent vectors
///
/// Time is given by the caller (milliseconds), so the GUI timer drives it and
/// tests can step it.
class GlideRamp
{
public:
    void start(std::vector<float> from, std::vector<float> to, double durationMs, double nowMs);

    /// @brief The vector for nowMs while gliding; the target on the last call, then nothing
    std::optional<std::vector<float>> tick(double nowMs);

    /// @brief Stops where it is; the last value given stays
    void cancel() noexcept
    {
        mRunning = false;
    }

    bool isRunning() const noexcept
    {
        return mRunning;
    }

    const std::vector<float> &target() const noexcept
    {
        return mTo;
    }

private:
    std::vector<float> mFrom;
    std::vector<float> mTo;
    double             mStartMs    = 0.0;
    double             mDurationMs = 0.0;
    bool               mRunning    = false;
};
