/// @file MapBackground.h
/// @brief What a map shows under its marks: a plain grid, the piece's cloud or the sound

#pragma once

#include <CloudImage.h>
#include <IMapView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <cstddef>
#include <vector>

/// @brief Paints a map's background for the layer it shows
///
/// Without a track it is a plain grid. With one it shows the piece's sliced
/// cloud, or the sound: a grid of decoded cells, bright where it is loud and
/// blue where it is tonal, with the cloud faintly on top.
class MapBackground
{
public:
    static constexpr float kCorner = 6.0f;

    void setPlain(bool plain);
    void setLayer(MapLayer layer);
    void setCloudLayout(std::vector<MapPoint> frames);
    void setCloudAlphas(std::vector<float> dots, std::vector<float> segments);

    /// @brief size × size cells, row by row from the bottom; size 0 clears it
    void setSound(std::size_t size, const std::vector<float> &brightness, const std::vector<float> &tonal);

    bool isPlain() const noexcept
    {
        return mPlain;
    }

    MapLayer layer() const noexcept
    {
        return mLayer;
    }

    /// @param origin Where the axes cross, in the same coordinates as area
    /// @param scale Physical pixels per logical pixel
    void paint(juce::Graphics &g, juce::Rectangle<float> area, juce::Point<float> origin, float scale);

private:
    void paintPlainGrid(juce::Graphics &g, juce::Rectangle<float> area) const;
    void paintSound(juce::Graphics &g, juce::Rectangle<float> area) const;

    CloudImage  mCloud;
    juce::Image mSound; ///< One pixel per cell, top row first
    MapLayer    mLayer = MapLayer::piece;
    bool        mPlain = true;
};
