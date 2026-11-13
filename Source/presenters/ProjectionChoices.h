/// @file ProjectionChoices.h
/// @brief The axis pairs a map's Projection menu offers

#pragma once

#include <MapAxes.h>

#include <algorithm>
#include <cstddef>
#include <vector>

/// @brief Principal pairs among the first four components (when there is a
/// track) followed by every pair of raw latent dims
class ProjectionChoices
{
public:
    static constexpr std::size_t kPrincipalComponents = 4;

    ProjectionChoices() = delete;

    static std::vector<MapAxes> forModel(std::size_t latentDim, bool hasTrack)
    {
        std::vector<MapAxes> choices;
        if (hasTrack)
        {
            appendPairs(choices, MapAxes::Space::principal, std::min(latentDim, kPrincipalComponents));
        }
        appendPairs(choices, MapAxes::Space::raw, latentDim);
        return choices;
    }

private:
    static void appendPairs(std::vector<MapAxes> &choices, MapAxes::Space space, std::size_t dims)
    {
        for (std::size_t x = 0; x < dims; ++x)
        {
            for (std::size_t y = x + 1; y < dims; ++y)
            {
                choices.push_back({space, x, y});
            }
        }
    }
};
