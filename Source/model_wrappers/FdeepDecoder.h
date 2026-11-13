/// @file FdeepDecoder.h
/// @brief Decoder running a frugally-deep (Keras) model

#pragma once

#include <IDecoder.h>

#include <fdeep/fdeep.hpp>

#include <string>

/// @brief IDecoder backed by fdeep::model
class FdeepDecoder : public IDecoder
{
public:
    /// @param modelJson The frugally-deep model as a JSON string
    explicit FdeepDecoder(const std::string &modelJson);

    std::size_t latentDim() const override;
    std::size_t outputDim() const override;
    void        decodeInto(const std::vector<float> &z, std::vector<float> &out) const override;

private:
    fdeep::model mModel;
    std::size_t  mLatentDim;
    std::size_t  mOutputDim;
};
