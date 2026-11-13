/// @file SnapshotBank.h
/// @brief Ten stored latent vectors the user can go back to

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

/// @brief Ten snapshot slots, each empty or holding a latent vector
///
/// Listeners hear about every store and clear. Message thread only.
class SnapshotBank
{
public:
    static constexpr std::size_t kSlots = 10;

    class Listener
    {
    public:
        virtual ~Listener()                                     = default;
        virtual void snapshotsChanged(const SnapshotBank &bank) = 0;
    };

    void store(std::size_t slot, const std::vector<float> &z);

    /// @brief The vector in slot, if any
    const std::optional<std::vector<float>> &at(std::size_t slot) const
    {
        return mSlots.at(slot);
    }

    /// @brief Empties every slot (a new model was loaded)
    void clear();

    void addListener(Listener *listener);
    void removeListener(Listener *listener);

private:
    void notify();

    std::array<std::optional<std::vector<float>>, kSlots> mSlots;
    std::vector<Listener *>                               mListeners;
};
