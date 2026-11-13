/// @file DecoderFactory.cpp
/// @brief Implementation of DecoderFactory

#include <DecoderFactory.h>

#include <FdeepDecoder.h>
#include <TorchDecoder.h>

std::unique_ptr<IDecoder> DecoderFactory::create(const DecoderSource &source)
{
    if (source.backend == ModelBackend::TorchScript)
    {
        return std::make_unique<TorchDecoder>(source.torchScriptPath, source.latentDim);
    }

    return std::make_unique<FdeepDecoder>(source.fdeepModelJson);
}
