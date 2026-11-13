/// @file LatentProjection.h
/// @brief Principal axes of a latent track, and moving between raw and principal coordinates

#pragma once

#include <AxisBounds.h>
#include <CoordTable.h>
#include <LatentTrack.h>

#include <Eigen/Dense>

#include <cstddef>
#include <span>
#include <vector>

/// @brief PCA of the piece's latent path
///
/// Components come by descending variance, each signed so the piece's long
/// tail points to + (right / up on a map). Principal coordinates are
/// c = Vᵀ (z − μ); standardized coordinates are (z − μ) / σ per raw dimension.
class LatentProjection
{
public:
    static LatentProjection fromTrack(const LatentTrack &track);

    std::size_t dims() const noexcept
    {
        return static_cast<std::size_t>(mMean.size());
    }

    /// @brief Raw z to principal coordinates
    std::vector<double> project(std::span<const float> z) const;

    /// @brief Principal coordinates back to raw z
    std::vector<float> unproject(std::span<const double> c) const;

    /// @brief Raw z to standardized coordinates
    std::vector<double> standardize(std::span<const float> z) const;

    /// @brief Share of the piece's variance along component k
    double varianceRatio(std::size_t k) const
    {
        return mRatios[static_cast<Eigen::Index>(k)];
    }

    /// @brief Map bounds of principal coordinate k
    const AxisBounds &bounds(std::size_t k) const
    {
        return mBounds.at(k);
    }

    /// @brief Every frame in principal coordinates
    const CoordTable &principalCoords() const noexcept
    {
        return mPrincipal;
    }

    /// @brief Every frame in standardized raw coordinates
    const CoordTable &standardizedCoords() const noexcept
    {
        return mStandardized;
    }

private:
    LatentProjection(
        Eigen::VectorXd mean, Eigen::VectorXd stdDev, Eigen::MatrixXd components, Eigen::VectorXd ratios,
        const Eigen::MatrixXd &centred
    );

    Eigen::VectorXd         mMean;
    Eigen::VectorXd         mStdDev;
    Eigen::MatrixXd         mComponents; ///< Column k is component k
    Eigen::VectorXd         mRatios;
    CoordTable              mPrincipal;
    CoordTable              mStandardized;
    std::vector<AxisBounds> mBounds;
};
