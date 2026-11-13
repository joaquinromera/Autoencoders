/// @file LoadedModel.h
/// @brief A model file turned into a ready engine plus what describes it

#pragma once

#include <Autoencoder.h>
#include <DecoderSource.h>
#include <LatentTrack.h>
#include <ModelDescriptor.h>

#include <memory>

/// @brief Result of loading a model file
struct LoadedModel
{
    std::unique_ptr<Autoencoder>       engine;
    ModelDescriptor                    descriptor;
    std::shared_ptr<const LatentTrack> track; ///< Null when the file has no ztrack
    DecoderSource                      decoderSource;
};
