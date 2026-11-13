/// @file BackgroundSubmitters.h
/// @brief Where the presenters send work for the background decoder

#pragma once

#include <SoundGrid.h>
#include <SweepJob.h>

#include <cstddef>
#include <functional>

/// @brief Destinations for sweep and sound grid jobs; either may be empty
struct BackgroundSubmitters
{
    std::function<void(const SweepJob &)>                      sweeps;
    std::function<void(std::size_t map, const SoundGridJob &)> soundGrids;
};
