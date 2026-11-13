/// @file MapLabels.cpp
/// @brief Implementation of MapLabels

#include <MapLabels.h>

#include <NearnessLabel.h>
#include <StringDefinitions.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
namespace Ui = Strings::Ui;

std::string axisName(const MapAxes &axes, std::size_t dim)
{
    const auto *prefix = axes.space == MapAxes::Space::principal ? Ui::PrincipalAxis : Ui::LatentAxis;
    return prefix + std::to_string(dim + 1);
}

int percentOf(const LatentProjection *projection, std::size_t k)
{
    return projection != nullptr ? static_cast<int>(std::lround(projection->varianceRatio(k) * 100.0)) : 0;
}

/// "+0.87" or "−1.52" with a real minus sign
std::string signedValue(double value)
{
    char digits[32];
    std::snprintf(digits, sizeof digits, "%.2f", std::abs(value));
    return std::string(value < 0.0 ? "−" : "+") + digits;
}
} // namespace

MapCaption MapLabels::caption(const MapAxes &axes, const LatentProjection *projection, MapPoint origin)
{
    MapCaption caption;
    caption.title  = axisName(axes, axes.x) + Ui::Times + axisName(axes, axes.y);
    caption.origin = origin;
    if (axes.space == MapAxes::Space::raw)
    {
        caption.subtitle = Ui::RawDims;
        caption.xLabel   = axisName(axes, axes.x);
        caption.yLabel   = axisName(axes, axes.y);
        return caption;
    }
    const int px     = percentOf(projection, axes.x);
    const int py     = percentOf(projection, axes.y);
    caption.subtitle = std::to_string(px + py) + Ui::OfVariance;
    caption.xLabel   = axisName(axes, axes.x) + Ui::Separator + std::to_string(px) + "%";
    caption.yLabel   = axisName(axes, axes.y) + Ui::Separator + std::to_string(py) + "%";
    return caption;
}

std::string MapLabels::menuEntry(const MapAxes &axes, const LatentProjection *projection)
{
    const auto title = axisName(axes, axes.x) + Ui::Times + axisName(axes, axes.y);
    if (axes.space == MapAxes::Space::raw)
    {
        return title;
    }
    return title + Ui::Separator + std::to_string(percentOf(projection, axes.x) + percentOf(projection, axes.y)) + "%";
}

std::string MapLabels::position(const MapAxes &axes, double x, double y)
{
    return axisName(axes, axes.x) + " " + signedValue(x) + "   " + axisName(axes, axes.y) + " " + signedValue(y);
}

MapNearness MapLabels::nearness(double nearness)
{
    const auto label = NearnessLabel::from(nearness);
    const auto gauge = static_cast<float>(std::clamp(nearness, 0.0, 1.0));
    switch (label.kind)
    {
        case NearnessLabel::Kind::on:
            return {MapNearness::Kind::on, Ui::OnThePiece, gauge};
        case NearnessLabel::Kind::near:
            return {MapNearness::Kind::near, Ui::NearThePiece + std::to_string(label.percent) + "%", gauge};
        case NearnessLabel::Kind::off:
            break;
    }
    return {MapNearness::Kind::off, Ui::OffThePiece, gauge};
}

std::vector<ProjectionItem>
MapLabels::menu(const std::vector<MapAxes> &choices, const MapAxes &selected, const LatentProjection *projection)
{
    std::vector<ProjectionItem> items;
    for (std::size_t id = 0; id < choices.size(); ++id)
    {
        const auto &choice = choices[id];
        items.push_back(
            {static_cast<int>(id),
             menuEntry(choice, projection),
             choice.space == MapAxes::Space::principal,
             choice == selected}
        );
    }
    return items;
}
