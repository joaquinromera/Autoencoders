/// @file LatentTrack.h
/// @brief The latent path of the audio a model was trained on

#pragma once

#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

/// @brief Frames of latent vectors, stored row-major (frame after frame)
class LatentTrack
{
public:
    /// @param latentDim Values per frame
    /// @param values frames × latentDim floats, row-major
    LatentTrack(std::size_t latentDim, std::vector<float> values)
        : mLatentDim(latentDim)
        , mValues(std::move(values))
    {
        if (latentDim == 0 || mValues.size() % latentDim != 0)
        {
            throw std::invalid_argument("LatentTrack values are not a whole number of frames");
        }
    }

    std::size_t latentDim() const noexcept
    {
        return mLatentDim;
    }

    std::size_t frames() const noexcept
    {
        return mValues.size() / mLatentDim;
    }

    /// @brief The latent vector of frame f
    std::span<const float> frame(std::size_t f) const
    {
        return std::span<const float>(mValues).subspan(f * mLatentDim, mLatentDim);
    }

    /// @brief All values, row-major
    const std::vector<float> &values() const noexcept
    {
        return mValues;
    }

private:
    std::size_t        mLatentDim;
    std::vector<float> mValues;
};
