/// @file MidiCcDefaults.h
/// @brief MIDI controller numbers a model file suggests for its controls

#pragma once

#include <cstddef>
#include <vector>

/// @brief Default CC per latent slider, plus the xMax and sClip CCs
struct MidiCcDefaults
{
    static constexpr int kFirstSliderCc = 1;
    static constexpr int kXMaxCc        = 16;
    static constexpr int kSClipCc       = 17;

    std::vector<int> sliderCcs; ///< One controller per latent dimension
    int              xMaxCc  = kXMaxCc;
    int              sClipCc = kSClipCc;

    /// @brief The assignment used when the file has none: CC 1..N, 16 and 17
    static MidiCcDefaults fallback(std::size_t latentDim)
    {
        MidiCcDefaults defaults;
        for (std::size_t i = 0; i < latentDim; ++i)
        {
            defaults.sliderCcs.push_back(kFirstSliderCc + static_cast<int>(i));
        }
        return defaults;
    }
};
