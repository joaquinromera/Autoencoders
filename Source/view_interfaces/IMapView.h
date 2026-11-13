/// @file IMapView.h
/// @brief One map: the piece's cloud, where you are, snapshot pins

#pragma once

#include <CcTagInfo.h>

#include <cstddef>
#include <string>
#include <vector>

/// @brief What a map's background shows
enum class MapLayer
{
    piece, ///< The piece's frames, sliced by the other sliders
    sound  ///< What the decoder makes at every point: level and tonality
};

/// @brief A point on the map, 0..1 left to right and bottom to top
struct MapPoint
{
    float x = 0.0f;
    float y = 0.0f;
};

/// @brief A snapshot slot drawn on the map
struct MapPin
{
    int      slot = 0; ///< 0..9, shown as 1..9, 0
    MapPoint at;
};

/// @brief Labels of a map
struct MapCaption
{
    std::string title;    ///< e.g. "PC1 × PC2"
    std::string subtitle; ///< e.g. "88% of the piece's variance"
    std::string xLabel;   ///< e.g. "PC1 · 76%"
    std::string yLabel;
    MapPoint    origin; ///< Where the axes cross (the piece's mean)
};

/// @brief The readout under a map's corner
struct MapNearness
{
    enum class Kind
    {
        on,
        near,
        off
    };

    Kind        kind = Kind::off;
    std::string text;         ///< e.g. "near the piece · 24%"
    float       gauge = 0.0f; ///< 0..1
};

/// @brief One entry of the Projection menu
struct ProjectionItem
{
    int         id = 0;
    std::string label; ///< e.g. "PC1 × PC2 · 88%" or "z1 × z3"
    bool        principal = true;
    bool        selected  = false;
};

/// @brief What the presenter tells a map
class IMapView
{
public:
    virtual ~IMapView() = default;

    /// @brief Where every frame of the piece sits; once per projection
    virtual void setCloudLayout(const std::vector<MapPoint> &frames) = 0;

    /// @brief Opacity of each frame dot, and of each segment joining frame f and f + 1
    virtual void showCloudAlphas(const std::vector<float> &dots, const std::vector<float> &segments) = 0;

    /// @brief Where you are; clamped to the edge and dashed when offMap
    virtual void showRing(MapPoint at, bool offMap) = 0;

    virtual void showPins(const std::vector<MapPin> &pins) = 0;
    virtual void showNearness(const MapNearness &nearness) = 0;

    /// @brief e.g. "PC1 −1.52   PC2 +0.87"
    virtual void showPosition(const std::string &text) = 0;

    virtual void showCaption(const MapCaption &caption) = 0;

    /// @brief true: no track, a plain pad without cloud or readout
    virtual void showPlainPad(bool plain) = 0;

    /// @brief The axes the Projection menu offers
    virtual void showProjectionMenu(const std::vector<ProjectionItem> &items) = 0;

    /// @brief Where a running glide goes (drawn as a dashed line from the ring)
    virtual void showGlideTarget(bool active, MapPoint at) = 0;

    /// @brief MIDI map mode tags, in the order x axis, y axis; empty hides them
    virtual void showCcTags(const std::vector<CcTagInfo> &tags) = 0;

    /// @brief Which background the map shows, and whether the sound one can be picked
    virtual void showLayer(MapLayer layer, bool soundAvailable) = 0;

    /// @brief The sound background: size × size cells, row by row from the bottom;
    /// size 0 clears it
    /// @param brightness 0..1 from the decoded level
    /// @param tonal 0..1 from noise to separate peaks
    virtual void
    showSoundGrid(std::size_t size, const std::vector<float> &brightness, const std::vector<float> &tonal) = 0;
};
