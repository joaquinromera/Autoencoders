/// @file MapLabels.h
/// @brief The texts a map shows: caption, position readout, nearness

#pragma once

#include <IMapView.h>
#include <LatentProjection.h>
#include <MapAxes.h>

#include <string>
#include <vector>

/// @brief Builds a map's texts from its axes
class MapLabels
{
public:
    MapLabels() = delete;

    /// @param projection Gives the variance shares of principal axes; may be null for raw axes
    static MapCaption caption(const MapAxes &axes, const LatentProjection *projection, MapPoint origin);

    /// @brief A Projection menu entry, e.g. "PC1 × PC2 · 88%" or "z1 × z3"
    static std::string menuEntry(const MapAxes &axes, const LatentProjection *projection);

    /// @brief The Projection menu: one entry per choice, its id the choice's index
    static std::vector<ProjectionItem>
    menu(const std::vector<MapAxes> &choices, const MapAxes &selected, const LatentProjection *projection);

    /// @brief e.g. "PC1 −1.52   PC2 +0.87"
    static std::string position(const MapAxes &axes, double x, double y);

    /// @brief The readout for a slice's nearness (1 = as on the piece)
    static MapNearness nearness(double nearness);
};
