/// @file LatentProjection.cpp
/// @brief Implementation of LatentProjection

#include <LatentProjection.h>

#include <algorithm>
#include <numeric>

namespace
{
Eigen::MatrixXd trackMatrix(const LatentTrack &track)
{
    Eigen::MatrixXd z(static_cast<Eigen::Index>(track.frames()), static_cast<Eigen::Index>(track.latentDim()));
    for (std::size_t f = 0; f < track.frames(); ++f)
    {
        const auto frame = track.frame(f);
        for (std::size_t i = 0; i < frame.size(); ++i)
        {
            z(static_cast<Eigen::Index>(f), static_cast<Eigen::Index>(i)) = frame[i];
        }
    }
    return z;
}

/// Eigenvectors of the covariance of centred, sorted by descending eigenvalue,
/// each flipped so the projection has positive skewness. Returns {V, ratios}.
std::pair<Eigen::MatrixXd, Eigen::VectorXd> orientedComponents(const Eigen::MatrixXd &centred)
{
    const Eigen::MatrixXd covariance = (centred.transpose() * centred) / (centred.rows() - 1);
    const Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(covariance);

    const auto      n = covariance.cols();
    Eigen::MatrixXd components(n, n);
    Eigen::VectorXd ratios(n);
    for (Eigen::Index k = 0; k < n; ++k)
    {
        Eigen::VectorXd      v    = solver.eigenvectors().col(n - 1 - k); // ascending → descending
        const Eigen::ArrayXd p    = (centred * v).array();
        const double         skew = (p - p.mean()).cube().mean();
        components.col(k)         = skew < 0 ? Eigen::VectorXd(-v) : v;
        ratios[k]                 = solver.eigenvalues()[n - 1 - k] / solver.eigenvalues().sum();
    }
    return {components, ratios};
}

CoordTable tableOf(const Eigen::MatrixXd &coords)
{
    std::vector<double> values(static_cast<std::size_t>(coords.size()));
    Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(
        values.data(),
        coords.rows(),
        coords.cols()
    ) = coords;
    return {static_cast<std::size_t>(coords.cols()), std::move(values)};
}

std::vector<AxisBounds> boundsOf(const Eigen::MatrixXd &coords)
{
    std::vector<AxisBounds> bounds;
    for (Eigen::Index k = 0; k < coords.cols(); ++k)
    {
        const Eigen::VectorXd column = coords.col(k);
        bounds.push_back(AxisBounds::robust({column.data(), column.data() + column.size()}));
    }
    return bounds;
}
} // namespace

LatentProjection LatentProjection::fromTrack(const LatentTrack &track)
{
    const Eigen::MatrixXd z       = trackMatrix(track);
    const Eigen::VectorXd mean    = z.colwise().mean();
    const Eigen::MatrixXd centred = z.rowwise() - mean.transpose();
    Eigen::VectorXd       stdDev  = (centred.array().square().colwise().mean()).sqrt();
    stdDev                        = (stdDev.array() > 0).select(stdDev, 1.0); // a constant dim stays as it is

    auto [components, ratios] = orientedComponents(centred);
    return {mean, stdDev, std::move(components), std::move(ratios), centred};
}

LatentProjection::LatentProjection(
    Eigen::VectorXd mean, Eigen::VectorXd stdDev, Eigen::MatrixXd components, Eigen::VectorXd ratios,
    const Eigen::MatrixXd &centred
)
    : mMean(std::move(mean))
    , mStdDev(std::move(stdDev))
    , mComponents(std::move(components))
    , mRatios(std::move(ratios))
    , mPrincipal(tableOf(centred * mComponents))
    , mStandardized(tableOf((centred.array().rowwise() / mStdDev.transpose().array()).matrix()))
    , mBounds(boundsOf(centred * mComponents))
{
}

std::vector<double> LatentProjection::project(std::span<const float> z) const
{
    const Eigen::VectorXd raw =
        Eigen::Map<const Eigen::VectorXf>(z.data(), static_cast<Eigen::Index>(z.size())).cast<double>();
    const Eigen::VectorXd c = mComponents.transpose() * (raw - mMean);
    return {c.data(), c.data() + c.size()};
}

std::vector<float> LatentProjection::unproject(std::span<const double> c) const
{
    const Eigen::VectorXd coords = Eigen::Map<const Eigen::VectorXd>(c.data(), static_cast<Eigen::Index>(c.size()));
    const Eigen::VectorXf z      = (mMean + (mComponents * coords)).cast<float>();
    return {z.data(), z.data() + z.size()};
}

std::vector<double> LatentProjection::standardize(std::span<const float> z) const
{
    std::vector<double> s(z.size());
    for (std::size_t i = 0; i < z.size(); ++i)
    {
        const auto ei = static_cast<Eigen::Index>(i);
        s[i]          = (z[i] - mMean[ei]) / mStdDev[ei];
    }
    return s;
}
