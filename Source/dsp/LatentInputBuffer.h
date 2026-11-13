/// @file LatentInputBuffer.h
/// @brief The latent vector the audio thread decodes, published whole

#pragma once

#include <LatestValueMailbox.h>

#include <cstddef>
#include <stdexcept>
#include <vector>

/// @brief Hands whole latent vectors from the message thread to the audio thread
///
/// Publishing the whole vector, rather than one element at a time, means the
/// audio thread always decodes a z that the user actually set.
class LatentInputBuffer
{
public:
    explicit LatentInputBuffer(std::size_t latentDim)
        : mMailbox(std::vector<float>(latentDim, 0.0f))
        , mCurrent(latentDim, 0.0f)
    {
    }

    /// @brief Message thread: makes z the next vector to decode
    /// @throws std::invalid_argument if z has the wrong size
    void publish(const std::vector<float> &z)
    {
        if (z.size() != mCurrent.size())
        {
            throw std::invalid_argument("Latent vector size does not match the model");
        }
        mMailbox.write(z);
    }

    /// @brief Audio thread: the latest published vector, or the previous one
    const std::vector<float> &current()
    {
        mMailbox.readLatest(mCurrent);
        return mCurrent;
    }

private:
    LatestValueMailbox<std::vector<float>> mMailbox;
    std::vector<float>                     mCurrent; ///< Owned by the audio thread
};
