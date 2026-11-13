#include <StartingLatent.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;

namespace
{
ControlRanges twoDimRanges()
{
    ControlRanges ranges;
    ranges.z = {
        { 0.0f, 10.0f},
        {-4.0f,  0.0f}
    };
    return ranges;
}
} // namespace

TEST(StartingLatent, PicksTrackFrameClosestToMean)
{
    // Mean is (2, -1); the middle frame is the closest real frame to it.
    const LatentTrack track(2, {0.0f, 0.0f, 2.5f, -1.0f, 3.5f, -2.0f});

    EXPECT_THAT(startingLatent(twoDimRanges(), &track), ElementsAre(2.5f, -1.0f));
}

TEST(StartingLatent, MidpointsWithoutTrack)
{
    EXPECT_THAT(startingLatent(twoDimRanges(), nullptr), ElementsAre(5.0f, -2.0f));
}

TEST(StartingLatent, MidpointsWhenTrackSizeDoesNotMatch)
{
    const LatentTrack track(3, {1.0f, 1.0f, 1.0f});

    EXPECT_THAT(startingLatent(twoDimRanges(), &track), ElementsAre(5.0f, -2.0f));
}
