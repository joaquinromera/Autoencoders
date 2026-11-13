/// @file IDecoder.h
/// @brief Turns a latent vector into one predicted spectrum frame

#pragma once

#include <cstddef>
#include <vector>

/// @brief A decoder maps a latent vector z to one log-power spectrum frame
///
/// Implementations are stateless: decoding depends only on the given z, so the
/// same instance can be called repeatedly without setup between calls. Each
/// thread that decodes (audio, background worker) owns its own instance.
class IDecoder
{
public:
    virtual ~IDecoder() = default;

    /// @brief Number of latent dimensions the decoder takes
    virtual std::size_t latentDim() const = 0;

    /// @brief Number of spectrum bins the decoder produces
    virtual std::size_t outputDim() const = 0;

    /// @brief Decodes z into out
    /// @param z Latent vector of size latentDim()
    /// @param out Receives outputDim() values; already sized buffers are reused
    virtual void decodeInto(const std::vector<float> &z, std::vector<float> &out) const = 0;
};
