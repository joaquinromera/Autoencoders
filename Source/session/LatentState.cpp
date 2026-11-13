/// @file LatentState.cpp
/// @brief Implementation of LatentState

#include <LatentState.h>

#include <algorithm>
#include <stdexcept>

void LatentState::reset(ControlRanges ranges, const std::vector<float> &z, float xMax, float sClip)
{
    mRanges = std::move(ranges);
    mZ.assign(mRanges.z.size(), 0.0f);
    assignClamped(z);
    mSClip = mRanges.clampSClip(sClip, mRanges.xMax.min);
    mXMax  = mRanges.clampXMax(xMax, mSClip);
    notify();
}

void LatentState::setZ(std::size_t dim, float value)
{
    if (dim >= mZ.size())
    {
        return;
    }

    const auto clamped = mRanges.clampZ(dim, value);
    if (clamped != mZ[dim])
    {
        mZ[dim] = clamped;
        notify();
    }
}

void LatentState::setLatent(const std::vector<float> &z)
{
    assignClamped(z);
    notify();
}

void LatentState::setXMax(float value)
{
    if (const auto clamped = mRanges.clampXMax(value, mSClip); clamped != mXMax)
    {
        mXMax = clamped;
        notify();
    }
}

void LatentState::setSClip(float value)
{
    if (const auto clamped = mRanges.clampSClip(value, mXMax); clamped != mSClip)
    {
        mSClip = clamped;
        notify();
    }
}

void LatentState::setBand(float xMax, float sClip)
{
    const auto newSClip = mRanges.clampSClip(sClip, mRanges.xMax.min);
    const auto newXMax  = mRanges.clampXMax(xMax, newSClip);
    if (newSClip != mSClip || newXMax != mXMax)
    {
        mSClip = newSClip;
        mXMax  = newXMax;
        notify();
    }
}

void LatentState::addListener(Listener *listener)
{
    if (std::find(mListeners.begin(), mListeners.end(), listener) == mListeners.end())
    {
        mListeners.push_back(listener);
    }
}

void LatentState::removeListener(Listener *listener)
{
    std::erase(mListeners, listener);
}

void LatentState::assignClamped(const std::vector<float> &z)
{
    if (z.size() != mZ.size())
    {
        throw std::invalid_argument("Latent vector size does not match the model");
    }
    for (std::size_t i = 0; i < z.size(); ++i)
    {
        mZ[i] = mRanges.clampZ(i, z[i]);
    }
}

void LatentState::notify()
{
    // Copy, so a listener may unsubscribe while being notified.
    const auto listeners = mListeners;
    for (auto *listener : listeners)
    {
        listener->latentStateChanged(*this);
    }
}
