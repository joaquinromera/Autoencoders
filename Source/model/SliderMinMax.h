/// @file SliderMinMax.h
/// @brief Simple structure for storing parameter ranges
/// @author Autoencoder Project
/// @date 2025

#pragma once

/// @brief Structure representing a parameter's minimum and maximum values
///
/// This simple structure is used to define the valid range for slider
/// controls and latent space parameters in the autoencoder interface.
struct SliderMinMax
{
    float min; ///< Minimum allowed value
    float max; ///< Maximum allowed value
};
