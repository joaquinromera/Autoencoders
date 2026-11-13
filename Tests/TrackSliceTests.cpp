#include <LatentProjection.h>
#include <SyntheticTrack.h>
#include <TrackSlice.h>

#include <gtest/gtest.h>

#include <cmath>

class TrackSliceTest : public ::testing::Test
{
protected:
    LatentTrack       track      = syntheticTrack();
    LatentProjection  projection = LatentProjection::fromTrack(track);
    const CoordTable &coords     = projection.principalCoords();
    TrackSlice        slice {
        coords,
        {0, 1}
    };
};

TEST_F(TrackSliceTest, WeightIsOneOnTheFrameItself)
{
    EXPECT_DOUBLE_EQ(slice.weightsAt(coords.row(42)).weights[42], 1.0);
}

TEST_F(TrackSliceTest, StandingOnAFrameGivesNearnessNearOne)
{
    // The reference is the median over frames, so a frame's nearness straddles 1.
    int above = 0;
    for (std::size_t f = 0; f < 200; ++f)
    {
        above += slice.nearness(slice.weightsAt(coords.row(f * 7)).mean) >= 1.0 ? 1 : 0;
    }
    EXPECT_GT(above, 60);
    EXPECT_LT(above, 140);
}

TEST_F(TrackSliceTest, FarFromThePieceIsBelowFivePercent)
{
    const std::vector<double> far {0.0, 0.0, 10.0, 10.0};

    EXPECT_LT(slice.nearness(slice.weightsAt(far).mean), 0.05);
}

TEST_F(TrackSliceTest, KeptCoordinatesDoNotChangeWeights)
{
    std::vector<double> c {0.0, 0.0, 0.1, -0.1};
    const auto          before = slice.weightsAt(c);
    c[0]                       = 25.0;
    c[1]                       = -25.0;
    const auto after           = slice.weightsAt(c);

    EXPECT_EQ(before.weights, after.weights);
}

TEST(TrackSlice, SigmaUsesUnseenDimsOnly)
{
    // Dim 0 (kept) has a huge spread, dim 1 (unseen) has mean square 1.
    const CoordTable coords(2, {1000.0, 1.0, -1000.0, -1.0});
    const TrackSlice slice(coords, {0});

    const auto   weights  = slice.weightsAt(std::vector<double> {0.0, 0.0});
    const double expected = std::exp(-0.5 / (0.35 * 0.35));
    EXPECT_NEAR(weights.weights[0], expected, 1e-12);
    EXPECT_NEAR(weights.weights[1], expected, 1e-12);
}

TEST(TrackSlice, NothingUnseenWeighsEveryFrameFully)
{
    const CoordTable coords(2, {1.0, 2.0, 3.0, 4.0});
    const TrackSlice slice(coords, {0, 1});

    EXPECT_DOUBLE_EQ(slice.weightsAt(std::vector<double> {9.0, 9.0}).mean, 1.0);
}
