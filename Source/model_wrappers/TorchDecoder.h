/// @file TorchDecoder.h
/// @brief Decoder running a TorchScript model

#pragma once

#include <IDecoder.h>

#include <torch/script.h>

#include <string>

/// @brief IDecoder backed by a TorchScript module
///
/// The module is not shared between threads: build one TorchDecoder per thread.
class TorchDecoder : public IDecoder
{
public:
    /// @param modelPath Path of the .pt file
    /// @param latentDim Latent size, used to probe the output size
    TorchDecoder(const std::string &modelPath, std::size_t latentDim);

    std::size_t latentDim() const override;
    std::size_t outputDim() const override;
    void        decodeInto(const std::vector<float> &z, std::vector<float> &out) const override;

private:
    at::Tensor forward(const at::Tensor &input) const;

    // forward() is not const in LibTorch, though inference leaves the module unchanged.
    mutable torch::jit::script::Module mModule;
    std::size_t                        mLatentDim;
    std::size_t                        mOutputDim = 0;
};
