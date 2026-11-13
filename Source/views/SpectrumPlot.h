/// @file SpectrumPlot.h
/// @brief The decoded spectrum between the Floor and Floor + Range lines, and the meter

#pragma once

#include <IOutputView.h>

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

/// @brief Draws the spectrum on a log frequency axis and lets the band be dragged
class SpectrumPlot : public juce::Component
{
public:
    /// @brief Called on a press, with its level and how near a line counts as on it
    std::function<void(float db, float reachDb)> onBandPressed;

    /// @brief Called while dragging, with the cursor's level
    std::function<void(float db)> onBandDragged;

    void setFrequencies(std::vector<double> hz);
    void setSpectrum(const std::vector<float> &db);
    void setBand(const OutputBand &band);
    void setMeter(float levelDb, float peakDb);

    void paint(juce::Graphics &g) override;
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;

private:
    juce::Rectangle<float> plotArea() const;
    float                  yOf(float db) const;
    float                  dbOf(float y) const;
    float                  xOfHz(double hz) const;
    void                   paintGrid(juce::Graphics &g) const;
    void                   paintSpectrum(juce::Graphics &g) const;
    void                   paintBand(juce::Graphics &g) const;
    void                   paintMeter(juce::Graphics &g) const;

    std::vector<double> mFrequencies;
    std::vector<float>  mSpectrum;
    OutputBand          mBand;
    float               mMeterDb = -120.0f;
    float               mPeakDb  = -120.0f;
};
