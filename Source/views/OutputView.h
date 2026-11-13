/// @file OutputView.h
/// @brief The output region: header with Floor and Range fields, and the spectrum plot

#pragma once

#include <CcTag.h>
#include <IOutputPresenter.h>
#include <IOutputView.h>
#include <SpectrumPlot.h>

#include <juce_gui_basics/juce_gui_basics.h>

/// @brief Output region of the main window
class OutputView
    : public juce::Component
    , public IOutputView
{
public:
    OutputView();

    void setPresenter(IOutputPresenter *presenter)
    {
        mPresenter = presenter;
    }

    void showFrequencies(const std::vector<double> &hz) override;
    void showSpectrum(const std::vector<float> &db) override;
    void showBand(const OutputBand &band) override;
    void showMeter(float levelDb, float peakDb) override;
    void showCcTags(const std::vector<CcTagInfo> &tags) override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    void setUpField(juce::Label &field, std::function<void(IOutputPresenter &, float)> onEdited);

    IOutputPresenter *mPresenter = nullptr;
    juce::Label       mFloorField;
    juce::Label       mRangeField;
    SpectrumPlot      mPlot;
    CcTag             mFloorTag; ///< MIDI map mode: the sClip controller
    CcTag             mRangeTag; ///< MIDI map mode: the xMax controller
};
