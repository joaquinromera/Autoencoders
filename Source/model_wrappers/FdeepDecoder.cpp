/// @file FdeepDecoder.cpp
/// @brief Implementation of FdeepDecoder

#include <FdeepDecoder.h>

FdeepDecoder::FdeepDecoder(const std::string &modelJson)
    : mModel(fdeep::read_model_from_string(modelJson, false, nullptr))
    , mLatentDim(mModel.get_input_shapes()[0].depth_.unsafe_get_just())
    , mOutputDim(mModel.get_output_shapes()[0].depth_.unsafe_get_just())
{
}

std::size_t FdeepDecoder::latentDim() const
{
    return mLatentDim;
}

std::size_t FdeepDecoder::outputDim() const
{
    return mOutputDim;
}

void FdeepDecoder::decodeInto(const std::vector<float> &z, std::vector<float> &out) const
{
    const fdeep::tensors input {fdeep::tensor(fdeep::tensor_shape {mLatentDim}, z)};
    const auto           result = mModel.predict(input)[0].as_vector();
    out.assign(result->begin(), result->end());
}
