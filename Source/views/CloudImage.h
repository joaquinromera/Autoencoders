/// @file CloudImage.h
/// @brief The sliced cloud of a map, rendered once and reused until it changes

#pragma once

#include <IMapView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <vector>

/// @brief Caches the cloud as an image: frame dots added up, path segments on top
class CloudImage
{
public:
    void setLayout(std::vector<MapPoint> frames);
    void setAlphas(std::vector<float> dots, std::vector<float> segments);
    void clear();

    bool isEmpty() const noexcept
    {
        return mFrames.empty() || mDots.size() != mFrames.size();
    }

    /// @brief The cloud at size (logical pixels) × scale; re-rendered only when changed
    const juce::Image &imageFor(juce::Rectangle<int> size, float scale);

    /// @brief A faint outline of every frame, for snapshot thumbnails
    static juce::Image renderOutline(const std::vector<MapPoint> &frames, int sizePx);

private:
    void render(juce::Rectangle<int> size, float scale);
    void drawSegments(juce::Graphics &g, float width, float height) const;
    void addDots(juce::Image &image, float width, float height, float scale) const;

    std::vector<MapPoint> mFrames;
    std::vector<float>    mDots;
    std::vector<float>    mSegments;
    juce::Image           mImage;
    juce::Rectangle<int>  mRenderedSize;
    float                 mRenderedScale = 0.0f;
    bool                  mDirty         = true;
};
