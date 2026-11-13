#include <LatentProjection.h>
#include <Percentile.h>
#include <SyntheticTrack.h>

#include <gtest/gtest.h>

#include <cmath>

namespace
{
std::vector<double> column(const CoordTable &table, std::size_t k)
{
    std::vector<double> values;
    for (std::size_t f = 0; f < table.frames(); ++f)
    {
        values.push_back(table.at(f, k));
    }
    return values;
}

double thirdMoment(const std::vector<double> &values)
{
    double sum = 0.0;
    for (const double v : values)
    {
        sum += v * v * v;
    }
    return sum / static_cast<double>(values.size());
}
} // namespace

class LatentProjectionTest : public ::testing::Test
{
protected:
    LatentTrack      track      = syntheticTrack();
    LatentProjection projection = LatentProjection::fromTrack(track);
};

TEST_F(LatentProjectionTest, ComponentsSortedByDescendingVariance)
{
    for (std::size_t k = 1; k < projection.dims(); ++k)
    {
        EXPECT_GE(projection.varianceRatio(k - 1), projection.varianceRatio(k));
    }
    EXPECT_GE(projection.principalCoords().meanSquares()[0], projection.principalCoords().meanSquares()[1]);
}

TEST_F(LatentProjectionTest, RatiosSumToOne)
{
    double sum = 0.0;
    for (std::size_t k = 0; k < projection.dims(); ++k)
    {
        sum += projection.varianceRatio(k);
    }
    EXPECT_NEAR(sum, 1.0, 1e-9);
}

TEST_F(LatentProjectionTest, ProjectThenUnprojectIsIdentity)
{
    const auto frame = track.frame(17);
    const auto back  = projection.unproject(projection.project(frame));

    for (std::size_t i = 0; i < frame.size(); ++i)
    {
        EXPECT_NEAR(back[i], frame[i], 1e-4);
    }
}

TEST_F(LatentProjectionTest, ComponentsOrientedToPositiveSkew)
{
    EXPECT_GT(thirdMoment(column(projection.principalCoords(), 0)), 0.0);

    // One step along the first component from the mean moves along (cos 30°, sin 30°).
    const auto origin = projection.unproject(std::vector<double> {0.0, 0.0, 0.0, 0.0});
    const auto step   = projection.unproject(std::vector<double> {1.0, 0.0, 0.0, 0.0});
    EXPECT_NEAR(step[0] - origin[0], std::cos(30.0 * 3.14159265 / 180.0), 0.05);
    EXPECT_NEAR(step[1] - origin[1], std::sin(30.0 * 3.14159265 / 180.0), 0.05);
}

TEST_F(LatentProjectionTest, BoundsCoverRobustPercentilesWithMargin)
{
    const auto values = column(projection.principalCoords(), 0);
    const auto lo     = percentile(values, 0.3);
    const auto hi     = percentile(values, 99.7);

    EXPECT_NEAR(projection.bounds(0).lo, lo - (0.08 * (hi - lo)), 1e-9);
    EXPECT_NEAR(projection.bounds(0).hi, hi + (0.08 * (hi - lo)), 1e-9);
}

TEST_F(LatentProjectionTest, StandardizedCoordsHaveUnitSpread)
{
    for (const double ms : projection.standardizedCoords().meanSquares())
    {
        EXPECT_NEAR(ms, 1.0, 1e-6);
    }
}

TEST(Percentile, MatchesNumpyLinearInterpolation)
{
    EXPECT_DOUBLE_EQ(percentile({4.0, 1.0, 3.0, 2.0}, 30.0), 1.9);
    EXPECT_DOUBLE_EQ(percentile({4.0, 1.0, 3.0, 2.0}, 100.0), 4.0);
}
