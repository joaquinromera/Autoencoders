/// @file ModelFileReader.h
/// @brief Reads a model JSON file into plain descriptions, without loading the network

#pragma once

#include <DecoderSource.h>
#include <LatentTrack.h>
#include <ModelDescriptor.h>

#include <memory>
#include <string>

/// @brief Everything a model file holds, still unloaded
struct ModelFile
{
    ModelDescriptor                    descriptor;
    DecoderSource                      decoderSource;
    std::shared_ptr<const LatentTrack> track; ///< Null when the file has no ztrack
};

/// @brief Parses model files exported by the training scripts
class ModelFileReader
{
public:
    ModelFileReader() = delete;

    /// @throws std::runtime_error if the file cannot be opened or lacks a model
    static ModelFile read(const std::string &path);

    /// @param jsonText Contents of a model file
    /// @param path Where the file lives; names the model and locates a .pt file
    /// @throws std::runtime_error if the text lacks a model
    static ModelFile parse(const std::string &jsonText, const std::string &path);
};
