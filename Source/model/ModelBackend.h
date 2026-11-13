/// @file ModelBackend.h
/// @brief Neural network runtimes a model file can be decoded with

#pragma once

/// @brief Enumeration of supported neural network backends
enum class ModelBackend
{
    FrugallyDeep, ///< TensorFlow/Keras via frugally-deep
    TorchScript   ///< PyTorch via LibTorch
};
