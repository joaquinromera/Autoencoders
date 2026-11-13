/// @file ModelDescriptor.h
/// @brief What a model file says about itself, apart from the network

#pragma once

#include <MidiCcDefaults.h>
#include <ModelBackend.h>
#include <SliderMinMax.h>

#include <cstddef>
#include <string>
#include <vector>

/// @brief Descriptive fields read from a model file
struct ModelDescriptor
{
    std::string               name; ///< File name without extension
    ModelBackend              backend    = ModelBackend::FrugallyDeep;
    std::size_t               latentDim  = 0;
    double                    sampleRate = 0.0; ///< Sample rate the model was trained at
    unsigned int              hopLength  = 0;
    unsigned int              winLength  = 0;
    std::vector<SliderMinMax> zRange; ///< Range of each latent dimension
    float                     defaultXMax  = 0.0f;
    float                     defaultSClip = 0.0f;
    std::string               sourceFile;            ///< Training audio, empty when unknown
    double                    durationSeconds = 0.0; ///< Length of the latent track
    MidiCcDefaults            ccDefaults;
};
