/// @file ILatentPanelView.h
/// @brief The latent sliders with their two strips each

#pragma once

#include <CcTagInfo.h>

#include <string>
#include <vector>

/// @brief One latent slider
struct LatentSliderSpec
{
    std::string label; ///< e.g. "z1"
    float       min = 0.0f;
    float       max = 1.0f;
};

/// @brief What the presenter tells the latent panel
class ILatentPanelView
{
public:
    virtual ~ILatentPanelView() = default;

    virtual void buildSliders(const std::vector<LatentSliderSpec> &sliders) = 0;

    /// @param flagged The value lies outside the piece: thumb drawn red
    virtual void showValue(int index, float value, bool flagged) = 0;

    /// @brief The piece's values of this dim given the others, √-scaled 0..1
    /// per bin, bottom to top; empty means nothing of the piece is left
    /// @param zeroShare Share of those frames with the dim at exactly 0, kept out of the bins
    virtual void showHistogram(int index, const std::vector<float> &heights, float zeroShare, bool empty) = 0;

    /// @brief What the decoder makes along the range, bottom to top
    /// @param brightness 0..1 from the decoded level
    /// @param tonal 0..1 from noise to separate peaks
    virtual void showSweep(int index, const std::vector<float> &brightness, const std::vector<float> &tonal) = 0;

    /// @brief The model has no track: sliders without strips
    virtual void showWithoutStrips() = 0;

    /// @brief MIDI map mode tags, in the order of the sliders; empty hides them
    virtual void showCcTags(const std::vector<CcTagInfo> &tags) = 0;
};
