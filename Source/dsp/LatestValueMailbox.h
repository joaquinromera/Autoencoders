/// @file LatestValueMailbox.h
/// @brief Lock-free hand-over of the latest value from one thread to another

#pragma once

#include <array>
#include <atomic>
#include <cstdint>

/// @brief Triple buffer: one writer thread, one reader thread, latest value wins
///
/// The writer fills its own slot and swaps it with the shared one; the reader
/// swaps the shared slot with its own when a fresh value is there. Neither side
/// waits for the other, and values are never torn. All three slots are copies
/// of the initial value, so writing values of the same size (e.g. vectors of
/// the same length) never allocates.
template <typename T>
class LatestValueMailbox
{
public:
    explicit LatestValueMailbox(const T &initial)
        : mSlots {initial, initial, initial}
    {
    }

    /// @brief Writer thread: publishes value, replacing any unread one
    void write(const T &value)
    {
        mSlots[mWriteSlot]  = value;
        const auto previous = mShared.exchange(mWriteSlot | kFreshBit, std::memory_order_acq_rel);
        mWriteSlot          = previous & kSlotMask;
    }

    /// @brief Reader thread: copies the latest value into out if one arrived
    /// @return false when nothing was written since the last read
    bool readLatest(T &out)
    {
        if ((mShared.load(std::memory_order_relaxed) & kFreshBit) == 0)
        {
            return false;
        }

        const auto previous = mShared.exchange(mReadSlot, std::memory_order_acq_rel);
        mReadSlot           = previous & kSlotMask;
        out                 = mSlots[mReadSlot];
        return true;
    }

private:
    static constexpr std::uint32_t kSlotMask = 0b011;
    static constexpr std::uint32_t kFreshBit = 0b100;

    std::array<T, 3>           mSlots;
    std::uint32_t              mWriteSlot = 0; ///< Owned by the writer
    std::uint32_t              mReadSlot  = 1; ///< Owned by the reader
    std::atomic<std::uint32_t> mShared {2};    ///< Slot in between, plus the fresh bit
};
