/// @file DecoderFactory.h
/// @brief Builds the decoder a DecoderSource describes

#pragma once

#include <DecoderSource.h>
#include <IDecoder.h>

#include <memory>

/// @brief Builds decoders from their source, once per thread that decodes
class DecoderFactory
{
public:
    DecoderFactory() = delete;

    /// @throws std::runtime_error if the model cannot be loaded
    static std::unique_ptr<IDecoder> create(const DecoderSource &source);
};
