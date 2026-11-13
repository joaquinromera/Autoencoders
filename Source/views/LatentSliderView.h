/// @file LatentSliderView.h
/// @brief One vertical latent slider with its sound strip (left) and piece strip (right)
///
/// The sound strip's brightness is the decoded level, its colour how tonal the
/// sound is. The piece strip marks the share of frames resting at exactly 0.

#pragma once

#include <CcTag.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <optional>
#include <vector>

/// @brief A vertical slider that draws what the piece and the decoder say about its range
class LatentSliderView : public juce::Component
{
public:
    LatentSliderView(juce::String label, float min, float max);

    std::function<void(float)> onDragged;
    std::function<void()>      onCcTagClicked;

    /// @brief Shows the MIDI tag, or hides it when tag is empty
    void setCcTag(std::optional<CcTagInfo> tag);
    void resized() override;

    void setValue(float value, bool flagged);
    void setHistogram(std::vector<float> heights, float zeroShare, bool empty);
    void setSweep(std::vector<float> brightness, std::vector<float> tonal);
    void setStripsVisible(bool visible);

    void paint(juce::Graphics &g) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;

private:
    juce::Rectangle<float> stripArea() const;
    float                  yOf(float value) const;
    float                  trackX() const;
    void                   paintSweep(juce::Graphics &g) const;
    void                   paintHistogram(juce::Graphics &g) const;
    void                   paintZeroShare(juce::Graphics &g) const;
    void                   paintTrackAndThumb(juce::Graphics &g) const;

    juce::String       mLabel;
    float              mMin;
    float              mMax;
    float              mValue   = 0.0f;
    bool               mFlagged = false;
    std::vector<float> mHeights;
    float              mZeroShare = 0.0f;
    bool               mEmpty     = false;
    std::vector<float> mSweep;
    std::vector<float> mTonal;
    bool               mStrips = true;
    CcTag              mTag; ///< MIDI map mode: this dim's controller
};
