/// @file SnapshotBank.cpp
/// @brief Implementation of SnapshotBank

#include <SnapshotBank.h>

#include <algorithm>

void SnapshotBank::store(std::size_t slot, const std::vector<float> &z)
{
    mSlots.at(slot) = z;
    notify();
}

void SnapshotBank::clear()
{
    mSlots.fill(std::nullopt);
    notify();
}

void SnapshotBank::addListener(Listener *listener)
{
    if (std::find(mListeners.begin(), mListeners.end(), listener) == mListeners.end())
    {
        mListeners.push_back(listener);
    }
}

void SnapshotBank::removeListener(Listener *listener)
{
    std::erase(mListeners, listener);
}

void SnapshotBank::notify()
{
    const auto listeners = mListeners;
    for (auto *listener : listeners)
    {
        listener->snapshotsChanged(*this);
    }
}
