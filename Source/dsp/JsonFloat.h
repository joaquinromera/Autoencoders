/// @file JsonFloat.h
/// @brief Reads a number that model files store either as a number or a string

#pragma once

#include <nlohmann/json.hpp>

#include <string>

/// @brief The float in value, which may be written as 1.5 or as "1.5"
inline float readJsonFloat(const nlohmann::json &value)
{
    return value.is_string() ? std::stof(value.get<std::string>()) : value.get<float>();
}
