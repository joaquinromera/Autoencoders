#include <BandHitTest.h>

#include <gtest/gtest.h>

// Top line at +40 dB, floor at −60 dB, 2 dB reach
TEST(BandHitTest, NearALineGrabsThatLine)
{
    EXPECT_EQ(BandHitTest::partAt(41.0f, 40.0f, -60.0f, 2.0f), BandPart::ceiling);
    EXPECT_EQ(BandHitTest::partAt(-61.5f, 40.0f, -60.0f, 2.0f), BandPart::floor);
}

TEST(BandHitTest, BetweenTheLinesGrabsTheBand)
{
    EXPECT_EQ(BandHitTest::partAt(0.0f, 40.0f, -60.0f, 2.0f), BandPart::band);
}

TEST(BandHitTest, OutsideTheBandGrabsNothing)
{
    EXPECT_FALSE(BandHitTest::partAt(50.0f, 40.0f, -60.0f, 2.0f).has_value());
    EXPECT_FALSE(BandHitTest::partAt(-70.0f, 40.0f, -60.0f, 2.0f).has_value());
}

TEST(BandHitTest, CloseLinesGoToTheNearerOne)
{
    // 3 dB apart, so a press between them is in reach of both
    EXPECT_EQ(BandHitTest::partAt(-59.5f, -57.0f, -60.0f, 2.0f), BandPart::floor);
    EXPECT_EQ(BandHitTest::partAt(-57.5f, -57.0f, -60.0f, 2.0f), BandPart::ceiling);
}

TEST(BandHitTest, LinesOnEachOtherSplitBySide)
{
    EXPECT_EQ(BandHitTest::partAt(-59.0f, -60.0f, -60.0f, 2.0f), BandPart::ceiling);
    EXPECT_EQ(BandHitTest::partAt(-61.0f, -60.0f, -60.0f, 2.0f), BandPart::floor);
}
