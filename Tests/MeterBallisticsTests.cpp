#include <MeterBallistics.h>

#include <gtest/gtest.h>

TEST(MeterBallistics, RisesAtOnce)
{
    MeterBallistics meter;
    meter.push(-12.0f, 0.0);

    EXPECT_FLOAT_EQ(meter.level(), -12.0f);
    EXPECT_FLOAT_EQ(meter.peak(), -12.0f);
}

TEST(MeterBallistics, DecaysAtConfiguredRate)
{
    MeterBallistics meter(20.0f, 500.0);
    meter.push(-10.0f, 0.0);

    meter.push(-60.0f, 250.0);

    EXPECT_NEAR(meter.level(), -15.0f, 1e-4f); // 20 dB/s for a quarter second
}

TEST(MeterBallistics, PeakHoldsThenFollows)
{
    MeterBallistics meter(20.0f, 500.0);
    meter.push(-10.0f, 0.0);

    meter.push(-60.0f, 400.0);
    EXPECT_FLOAT_EQ(meter.peak(), -10.0f);

    meter.push(-60.0f, 600.0);
    EXPECT_FLOAT_EQ(meter.peak(), meter.level());
}
