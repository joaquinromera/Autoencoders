/// @file CoordTable.h
/// @brief Every frame of the latent track in one coordinate system

#pragma once

#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

/// @brief frames × dims coordinates, row-major, with each column's mean square
///
/// The mean squares are the spread of the piece along each coordinate (the
/// coordinates are centred), which sets how wide a slice is.
class CoordTable
{
public:
    CoordTable(std::size_t dims, std::vector<double> values)
        : mDims(dims)
        , mValues(std::move(values))
        , mMeanSquares(dims, 0.0)
    {
        if (dims == 0 || mValues.empty() || mValues.size() % dims != 0)
        {
            throw std::invalid_argument("CoordTable values are not a whole number of frames");
        }
        for (std::size_t i = 0; i < mValues.size(); ++i)
        {
            mMeanSquares[i % dims] += mValues[i] * mValues[i] / static_cast<double>(frames());
        }
    }

    std::size_t frames() const noexcept
    {
        return mValues.size() / mDims;
    }

    std::size_t dims() const noexcept
    {
        return mDims;
    }

    double at(std::size_t frame, std::size_t dim) const
    {
        return mValues[(frame * mDims) + dim];
    }

    std::span<const double> row(std::size_t frame) const
    {
        return std::span<const double>(mValues).subspan(frame * mDims, mDims);
    }

    const std::vector<double> &meanSquares() const noexcept
    {
        return mMeanSquares;
    }

private:
    std::size_t         mDims;
    std::vector<double> mValues;
    std::vector<double> mMeanSquares;
};
