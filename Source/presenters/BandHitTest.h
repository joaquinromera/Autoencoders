/// @file BandHitTest.h
/// @brief Which part of the Floor/Range band a press grabs

#pragma once

#include <IOutputPresenter.h>

#include <algorithm>
#include <cmath>
#include <optional>

/// @brief Picks the band part under a press, in dB
class BandHitTest
{
public:
    BandHitTest() = delete;

    /// @brief The nearer line within reach wins; when the lines sit on each
    /// other, a press above them takes the top and one below the floor.
    /// Between the lines, out of reach of both, it is the band.
    static std::optional<BandPart> partAt(float db, float topDb, float floorDb, float reachDb)
    {
        const float toTop   = std::abs(db - topDb);
        const float toFloor = std::abs(db - floorDb);
        if (std::min(toTop, toFloor) <= reachDb)
        {
            if (toTop == toFloor)
            {
                return db >= topDb ? BandPart::ceiling : BandPart::floor;
            }
            return toTop < toFloor ? BandPart::ceiling : BandPart::floor;
        }
        if (db > floorDb && db < topDb)
        {
            return BandPart::band;
        }
        return std::nullopt;
    }
};
