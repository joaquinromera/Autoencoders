/// @file TorchDecoder.cpp
/// @brief Implementation of TorchDecoder

#include <TorchDecoder.h>

#include <juce_core/juce_core.h>
#include <StringDefinitions.h>

#include <ATen/Parallel.h>

#include <stdexcept>

namespace
{
torch::jit::script::Module loadModule(const std::string &modelPath)
{
    try
    {
        auto module = torch::jit::load(modelPath);
        module.eval();
        return module;
    }
    catch (const c10::Error &e)
    {
        throw std::runtime_error(
            std::string(Strings::Errors::ErrorLoadingTorchScriptModel) + modelPath + ": " + e.what()
        );
    }
}
} // namespace

TorchDecoder::TorchDecoder(const std::string &modelPath, std::size_t latentDim)
    : mModule(loadModule(modelPath))
    , mLatentDim(latentDim)
{
    // One intra-op thread: the decoder runs on the audio thread, and a pool of
    // LibTorch workers would compete with it for the cores.
    at::set_num_threads(1);

    mOutputDim = static_cast<std::size_t>(forward(torch::zeros({1, static_cast<int64_t>(latentDim)})).size(1));

    DBG(Strings::Debug::TorchWrapper::ModelLoadedSuccessfully);
    DBG(Strings::Debug::TorchWrapper::InputDepthPrefix << mLatentDim);
    DBG(Strings::Debug::TorchWrapper::OutputDepthPrefix << mOutputDim);
}

std::size_t TorchDecoder::latentDim() const
{
    return mLatentDim;
}

std::size_t TorchDecoder::outputDim() const
{
    return mOutputDim;
}

void TorchDecoder::decodeInto(const std::vector<float> &z, std::vector<float> &out) const
{
    // from_blob wraps z without copying; forward() only reads it.
    auto input =
        torch::from_blob(const_cast<float *>(z.data()), {1, static_cast<int64_t>(mLatentDim)}, torch::kFloat32);
    const auto output = forward(input).contiguous();
    const auto values = output.data_ptr<float>();
    out.assign(values, values + mOutputDim);
}

at::Tensor TorchDecoder::forward(const at::Tensor &input) const
{
    try
    {
        const torch::NoGradGuard noGrad;
        return mModule.forward({input}).toTensor();
    }
    catch (const c10::Error &e)
    {
        throw std::runtime_error(std::string(Strings::Errors::ErrorDuringModelPrediction) + e.what());
    }
}
