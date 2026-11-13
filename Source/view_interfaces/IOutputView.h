/// @file IOutputView.h
/// @brief The output region: decoded spectrum, Floor/Range band and meter

#pragma once

#include <CcTagInfo.h>

#include <vector>

/// @brief Floor (sClip) and Range (xMax) as the output region shows them
struct OutputBand
{
    float floorDb        = 0.0f;  ///< sClip
    float rangeDb        = 0.0f;  ///< xMax; the band's top is floor + range
    bool  floorAtDefault = false; ///< Equal to the model file's default
    bool  rangeAtDefault = false;
    float lowestDb       = -120.0f; ///< The lowest floor: the plot's bottom
    float highestDb      = 60.0f;   ///< The highest top: the plot's top
};

/// @brief What the presenter tells the output region
class IOutputView
{
public:
    virtual ~IOutputView() = default;

    /// @brief Frequencies of the spectrum points, once per model
    virtual void showFrequencies(const std::vector<double> &hz) = 0;

    /// @brief The spectrum being played, in dB, one value per frequency
    virtual void showSpectrum(const std::vector<float> &db) = 0;

    virtual void showBand(const OutputBand &band) = 0;

    /// @brief Output level and held peak in dBFS
    virtual void showMeter(float levelDb, float peakDb) = 0;

    /// @brief MIDI map mode tags, in the order Floor, Range; empty hides them
    virtual void showCcTags(const std::vector<CcTagInfo> &tags) = 0;
};
