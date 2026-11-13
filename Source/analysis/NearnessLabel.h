/// @file NearnessLabel.h
/// @brief How a map describes its slice: on, near or off the piece

#pragma once

#include <cmath>

/// @brief The readout under a map
struct NearnessLabel
{
    enum class Kind
    {
        on,   ///< ≥ 50 % of what standing on the piece gives
        near, ///< 5 – 50 %
        off   ///< < 5 %
    };

    Kind kind    = Kind::off;
    int  percent = 0; ///< Rounded nearness, shown when near

    static NearnessLabel from(double nearness)
    {
        const int percent = static_cast<int>(std::lround(nearness * 100.0));
        if (nearness >= 0.5)
        {
            return {Kind::on, percent};
        }
        return {nearness >= 0.05 ? Kind::near : Kind::off, percent};
    }
};
