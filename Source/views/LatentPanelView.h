/// @file LatentPanelView.h
/// @brief The latent region: header with the strip key, and one slider per dimension

#pragma once

#include <ILatentPanelPresenter.h>
#include <ILatentPanelView.h>
#include <LatentSliderView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

/// @brief Latent region of the main window
class LatentPanelView
    : public juce::Component
    , public ILatentPanelView
{
public:
    void setPresenter(ILatentPanelPresenter *presenter)
    {
        mPresenter = presenter;
    }

    void buildSliders(const std::vector<LatentSliderSpec> &sliders) override;
    void showValue(int index, float value, bool flagged) override;
    void showHistogram(int index, const std::vector<float> &heights, float zeroShare, bool empty) override;
    void showSweep(int index, const std::vector<float> &brightness, const std::vector<float> &tonal) override;
    void showWithoutStrips() override;
    void showCcTags(const std::vector<CcTagInfo> &tags) override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    LatentSliderView *sliderAt(int index) const;

    ILatentPanelPresenter                         *mPresenter = nullptr;
    std::vector<std::unique_ptr<LatentSliderView>> mSliders;
};
