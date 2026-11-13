#include <GlideRamp.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;

TEST(GlideRamp, InterpolatesLinearly)
{
    GlideRamp ramp;
    ramp.start({0.0f, 10.0f}, {1.0f, 0.0f}, 200.0, 1'000.0);

    EXPECT_THAT(*ramp.tick(1'050.0), ElementsAre(0.25f, 7.5f));
    EXPECT_THAT(*ramp.tick(1'100.0), ElementsAre(0.5f, 5.0f));
    EXPECT_THAT(*ramp.tick(1'250.0), ElementsAre(1.0f, 0.0f));
    EXPECT_FALSE(ramp.isRunning());
    EXPECT_FALSE(ramp.tick(1'300.0).has_value());
}

TEST(GlideRamp, ZeroDurationJumps)
{
    GlideRamp ramp;
    ramp.start({0.0f}, {3.0f}, 0.0, 0.0);

    EXPECT_THAT(*ramp.tick(0.0), ElementsAre(3.0f));
    EXPECT_FALSE(ramp.isRunning());
}

TEST(GlideRamp, CancelHoldsCurrentValue)
{
    GlideRamp ramp;
    ramp.start({0.0f}, {4.0f}, 100.0, 0.0);
    ramp.tick(50.0);

    ramp.cancel();

    EXPECT_FALSE(ramp.isRunning());
    EXPECT_FALSE(ramp.tick(100.0).has_value());
}
