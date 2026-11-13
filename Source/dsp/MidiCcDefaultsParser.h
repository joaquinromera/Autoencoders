/// @file MidiCcDefaultsParser.h
/// @brief Reads the CC assignment a model file suggests

#pragma once

#include <MidiCcDefaults.h>

#include <nlohmann/json.hpp>

#include <cstddef>

/// @brief CCs from the "automation" section, or MidiCcDefaults::fallback
///
/// Falls back as a whole when the section is missing or its slider list does
/// not have one CC per latent dimension; a missing xMax/sClip CC keeps 16/17.
MidiCcDefaults parseMidiCcDefaults(const nlohmann::json &root, std::size_t latentDim);
