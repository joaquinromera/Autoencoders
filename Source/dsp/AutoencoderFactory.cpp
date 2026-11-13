/// @file AutoencoderFactory.cpp
/// @brief Implementation of the AutoencoderFactory class

#include <AutoencoderFactory.h>

#include <DecoderFactory.h>
#include <ModelFileReader.h>

#include <stdexcept>

LoadedModel AutoencoderFactory::createFromFile(const std::string &path)
{
    auto file    = ModelFileReader::read(path);
    auto decoder = DecoderFactory::create(file.decoderSource);

    const auto &d = file.descriptor;
    if (decoder->latentDim() != d.latentDim || d.zRange.size() != d.latentDim)
    {
        throw std::runtime_error("Model input size does not match latent_dim and zRange");
    }

    auto engine = std::make_unique<Autoencoder>(
        std::move(decoder),
        Parameters {d.winLength, d.defaultXMax, d.defaultSClip},
        d.backend
    );
    return {std::move(engine), std::move(file.descriptor), std::move(file.track), std::move(file.decoderSource)};
}
