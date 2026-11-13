/// @file DecodedLevel.h
/// @brief How loud a decoded spectrum is, through the engine's dB mapping

#pragma once

#include <span>

/// @brief Total power of one decoded frame in dB
///
/// Each bin's power is 10^((pred·xMax + sClip) / 10), as the engine scales it.
double levelDb(std::span<const float> pred, float xMax, float sClip);

/// @brief A range of levels, low to high, in dB
struct LevelRange
{
    double lo = 0.0;
    double hi = 0.0;
};

/// @brief 0..1 brightness of level against the piece's levels
///
/// 1 at range.hi, 0 at 15 dB under range.lo, so quiet parts of the piece
/// still show and silence goes dark.
double brightness(double level, const LevelRange &range);
