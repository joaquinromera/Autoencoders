/// @file UnseenWatch.h
/// @brief Whether the coordinates a map does not show have moved

#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

/// @brief Remembers a map's unseen coordinates and tells when they move
///
/// What a map draws from the piece, and its sound background, depend only on
/// the coordinates it does not show; dragging on the map itself leaves them.
class UnseenWatch
{
public:
    /// Closer than this counts as unchanged (clamping after a drag on the map
    /// can nudge them by rounding)
    static constexpr double kTolerance = 1e-4;

    /// @brief The next check reports a move whatever the coordinates
    void forget()
    {
        mLast.clear();
    }

    /// @brief Whether any unseen coordinate moved since the last reported move;
    /// remembers coords when it did
    bool moved(const std::vector<double> &coords, const std::vector<std::size_t> &unseen)
    {
        if (mLast.size() == coords.size() && !anyMoved(coords, unseen))
        {
            return false;
        }
        mLast = coords;
        return true;
    }

private:
    bool anyMoved(const std::vector<double> &coords, const std::vector<std::size_t> &unseen) const
    {
        for (const auto k : unseen)
        {
            if (std::abs(coords[k] - mLast[k]) > kTolerance)
            {
                return true;
            }
        }
        return false;
    }

    std::vector<double> mLast;
};
