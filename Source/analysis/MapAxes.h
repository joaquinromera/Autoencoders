/// @file MapAxes.h
/// @brief Which two coordinates a map plots

#pragma once

#include <cstddef>

/// @brief Two principal components, or two raw latent dimensions
struct MapAxes
{
    enum class Space
    {
        principal,
        raw
    };

    Space       space = Space::principal;
    std::size_t x     = 0;
    std::size_t y     = 1;

    bool operator==(const MapAxes &) const = default;
};
