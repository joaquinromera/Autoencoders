/// @file DecodedTonality.h
/// @brief How tonal a decoded spectrum is: separate peaks or flat noise

#pragma once

#include <span>

/// @brief 1 − spectral flatness of one decoded frame's magnitudes
///
/// Flatness is the geometric over the arithmetic mean of the magnitudes
/// 10^((pred·xMax + sClip) / 20), without the DC bin. White noise is close
/// to 0, a few pure tones close to 1. Unlike the level, it does not change
/// when the whole spectrum gets louder or quieter.
double tonality(std::span<const float> pred, float xMax, float sClip);

/// @brief 0..1 strength a strip gives a tonality
///
/// The noisiest decoded frames sit around 0.15 and the most tonal ones above
/// 0.9, so that span is stretched to the whole 0..1.
double tonalStrength(double tonality);
