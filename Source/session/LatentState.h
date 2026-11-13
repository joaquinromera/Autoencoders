/// @file LatentState.h
/// @brief The one place that holds where the user is: z, xMax and sClip

#pragma once

#include <ControlRanges.h>

#include <cstddef>
#include <vector>

/// @brief Owns the latent vector, xMax and sClip of the loaded model
///
/// Sliders, pads, presets, MIDI and model loading all write through here;
/// values are clamped to the model's ranges. Listeners hear about every change
/// with the whole state, so nothing downstream keeps a partial copy.
/// Message thread only.
class LatentState
{
public:
    /// @brief Hears about every change of the state
    class Listener
    {
    public:
        virtual ~Listener()                                       = default;
        virtual void latentStateChanged(const LatentState &state) = 0;
    };

    /// @brief Starts over with a model's ranges and starting values
    /// @throws std::invalid_argument if z does not have one value per range
    void reset(ControlRanges ranges, const std::vector<float> &z, float xMax, float sClip);

    /// @brief Moves one latent dimension; out-of-range dims are ignored
    void setZ(std::size_t dim, float value);

    /// @brief Moves the whole latent vector
    /// @throws std::invalid_argument if z has the wrong size
    void setLatent(const std::vector<float> &z);

    void setXMax(float value);
    void setSClip(float value);

    /// @brief Moves xMax and sClip together, with one notification
    void setBand(float xMax, float sClip);

    const std::vector<float> &latent() const noexcept
    {
        return mZ;
    }

    float xMax() const noexcept
    {
        return mXMax;
    }

    float sClip() const noexcept
    {
        return mSClip;
    }

    const ControlRanges &ranges() const noexcept
    {
        return mRanges;
    }

    void addListener(Listener *listener);
    void removeListener(Listener *listener);

private:
    void assignClamped(const std::vector<float> &z);
    void notify();

    ControlRanges           mRanges;
    std::vector<float>      mZ;
    float                   mXMax  = 0.0f;
    float                   mSClip = 0.0f;
    std::vector<Listener *> mListeners;
};
