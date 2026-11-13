/// @file DecoderSource.h
/// @brief Everything needed to build a decoder instance again

#pragma once

#include <ModelBackend.h>

#include <cstddef>
#include <string>

/// @brief Where a model's decoder comes from
///
/// Kept after loading so a second decoder (for a background thread) can be
/// built without reading the model file again.
struct DecoderSource
{
    ModelBackend backend = ModelBackend::FrugallyDeep;
    std::string  fdeepModelJson;  ///< Embedded frugally-deep model (FrugallyDeep only)
    std::string  torchScriptPath; ///< Path of the .pt file (TorchScript only)
    std::size_t  latentDim = 0;   ///< Latent size, needed by TorchScript to probe the model
};
