/// @file GlideRamp.cpp
/// @brief Implementation of GlideRamp

#include <GlideRamp.h>

#include <algorithm>

void GlideRamp::start(std::vector<float> from, std::vector<float> to, double durationMs, double nowMs)
{
    mFrom       = std::move(from);
    mTo         = std::move(to);
    mStartMs    = nowMs;
    mDurationMs = std::max(0.0, durationMs);
    mRunning    = mFrom.size() == mTo.size();
}

std::optional<std::vector<float>> GlideRamp::tick(double nowMs)
{
    if (!mRunning)
    {
        return std::nullopt;
    }
    const double t = mDurationMs > 0.0 ? std::clamp((nowMs - mStartMs) / mDurationMs, 0.0, 1.0) : 1.0;
    if (t >= 1.0)
    {
        mRunning = false;
        return mTo;
    }
    std::vector<float> value(mTo.size());
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        value[i] = mFrom[i] + (static_cast<float>(t) * (mTo[i] - mFrom[i]));
    }
    return value;
}
