/// @file Parameters.h
/// @brief Parameter structure for autoencoder configuration
/// @author Autoencoder Project
/// @date 2025

#pragma once

/// @brief Configuration parameters for the Autoencoder
///
/// The window length sets the FFT size; xMax and sClip are the values the
/// engine plays with until the session sets its own.
struct Parameters
{
    /// @brief Constructs a Parameters object with the specified values
    /// @param pWinLength The window length for FFT operations
    /// @param pXMax Initial magnitude scaling
    /// @param pSClip Initial spectral floor in dB
    Parameters(unsigned int pWinLength, float pXMax, float pSClip);

    unsigned int winLength; ///< Window length for FFT operations (must be power of 2)
    float        xMax;      ///< Initial magnitude scaling
    float        sClip;     ///< Initial spectral floor in dB
};
