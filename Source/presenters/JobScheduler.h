/// @file JobScheduler.h
/// @brief Submits background decoding jobs once their inputs settle

#pragma once

#include <cstdint>
#include <functional>
#include <optional>

/// @brief Debounces job requests and tags them with generations
///
/// request() remembers the latest job; tick() submits it once nothing new
/// arrived for 100 ms. Every submission gets a higher generation, so results
/// of older submissions can be recognised and dropped. Job needs a
/// `std::uint64_t generation` member.
template <typename Job>
class JobScheduler
{
public:
    static constexpr double kDebounceMs = 100.0;

    using Clock     = std::function<double()>; ///< Milliseconds, monotonic
    using Submitter = std::function<void(const Job &)>;

    explicit JobScheduler(Clock clock)
        : mClock(std::move(clock))
    {
    }

    /// @brief Where submissions go; none means requests are dropped
    void setSubmitter(Submitter submitter)
    {
        mSubmitter = std::move(submitter);
        mPending.reset();
    }

    void request(Job job)
    {
        mPending     = std::move(job);
        mRequestedAt = mClock();
    }

    /// @brief Submits the pending request if it has been quiet long enough
    void tick()
    {
        if (!mPending || !mSubmitter || mClock() - mRequestedAt < kDebounceMs)
        {
            return;
        }
        mPending->generation = ++mGeneration;
        mSubmitter(*mPending);
        mPending.reset();
    }

    /// @brief Drops the pending request and makes every submitted one stale
    void invalidate()
    {
        mPending.reset();
        ++mGeneration;
    }

    /// @brief Whether generation is the latest submitted (and not invalidated since)
    bool isLatest(std::uint64_t generation) const noexcept
    {
        return generation == mGeneration;
    }

private:
    Clock              mClock;
    Submitter          mSubmitter;
    std::optional<Job> mPending;
    double             mRequestedAt = 0.0;
    std::uint64_t      mGeneration  = 0;
};
