/// @file AutoencoderFactory.h
/// @brief Factory class for creating Autoencoder instances from configuration files
/// @author Autoencoder Project
/// @date 2025

#pragma once

#include <LoadedModel.h>

#include <string>

/// @brief Builds a ready engine from a model file
class AutoencoderFactory
{
public:
    AutoencoderFactory() = delete;

    /// @brief Reads the model file and loads its decoder into a new engine
    /// @param path The file system path to the JSON configuration file
    /// @throws std::runtime_error if the file cannot be opened, parsed or loaded
    static LoadedModel createFromFile(const std::string &path);
};
