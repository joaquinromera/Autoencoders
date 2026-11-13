/// @file MeterBallistics.h
/// @brief How the output meter rises, falls and holds its peak

#pragma once

#include <algorithm>

/// @brief Meter level with instant attack and steady decay, and a held peak
class MeterBallistics
{
public:
    static constexpr float  kDefaultDecayDbPerSecond = 24.0f;
    static constexpr double kDefaultHoldMs           = 1'000.0;
    static constexpr float  kFloorDb                 = -120.0f;

    explicit MeterBallistics(float decayDbPerSecond = kDefaultDecayDbPerSecond, double holdMs = kDefaultHoldMs)
        : mDecayDbPerMs(decayDbPerSecond / 1'000.0f)
        , mHoldMs(holdMs)
    {
    }

    /// @brief A new reading at nowMs (milliseconds, monotonic)
    void push(float levelDb, double nowMs)
    {
        const auto elapsed = static_cast<float>(std::max(0.0, nowMs - mLastMs));
        mLastMs            = nowMs;
        mLevel             = std::max(levelDb, std::max(kFloorDb, mLevel - (mDecayDbPerMs * elapsed)));
        if (levelDb >= mPeak || nowMs - mPeakAtMs > mHoldMs)
        {
            mPeak     = std::max(levelDb, mLevel);
            mPeakAtMs = nowMs;
        }
    }

    float level() const noexcept
    {
        return mLevel;
    }

    float peak() const noexcept
    {
        return mPeak;
    }

private:
    float  mDecayDbPerMs;
    double mHoldMs;
    float  mLevel    = kFloorDb;
    float  mPeak     = kFloorDb;
    double mLastMs   = 0.0;
    double mPeakAtMs = 0.0;
};
