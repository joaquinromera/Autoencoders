/// @file LatentTrackParser.h
/// @brief Reads the ztrack section of a model file

#pragma once

#include <LatentTrack.h>

#include <nlohmann/json.hpp>

#include <cstddef>
#include <memory>

/// @brief The latent track stored under "ztrack"
/// @return Null when the file has no ztrack, it is empty, or a frame has the wrong size
std::shared_ptr<const LatentTrack> parseLatentTrack(const nlohmann::json &root, std::size_t latentDim);
