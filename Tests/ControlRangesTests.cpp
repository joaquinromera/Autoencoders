#include <ControlRanges.h>

#include <gtest/gtest.h>

namespace
{
ModelDescriptor descriptorWith(float defaultXMax, float defaultSClip)
{
    ModelDescriptor d;
    d.zRange = {
        {-0.2f, 4.1f}
    };
    d.defaultXMax  = defaultXMax;
    d.defaultSClip = defaultSClip;
    return d;
}
} // namespace

TEST(ControlRanges, XMaxSpansFromTheLowestFloorToTheHighestTop)
{
    const auto ranges = ControlRanges::forModel(descriptorWith(107.55f, -60.0f));

    EXPECT_FLOAT_EQ(ranges.xMax.min, 0.0f);
    EXPECT_FLOAT_EQ(ranges.xMax.max, 190.0f); // −120 to +70
}

TEST(ControlRanges, SClipRangeCoversDefault)
{
    const auto usual = ControlRanges::forModel(descriptorWith(100.0f, -60.0f)).sClip;
    const auto deep  = ControlRanges::forModel(descriptorWith(100.0f, -135.0f)).sClip;

    EXPECT_FLOAT_EQ(usual.min, -120.0f);
    EXPECT_FLOAT_EQ(usual.max, 0.0f);
    EXPECT_FLOAT_EQ(deep.min, -140.0f);
}

TEST(ControlRanges, ZRangeComesFromTheFile)
{
    const auto ranges = ControlRanges::forModel(descriptorWith(100.0f, -60.0f));

    ASSERT_EQ(ranges.z.size(), 1U);
    EXPECT_FLOAT_EQ(ranges.clampZ(0, 9.0f), 4.1f);
}

TEST(ControlRanges, TopIs20DbOverTheDefaultTop)
{
    EXPECT_FLOAT_EQ(ControlRanges::forModel(descriptorWith(100.0f, -60.0f)).topMax, 60.0f);
    EXPECT_FLOAT_EQ(ControlRanges::forModel(descriptorWith(107.55f, -60.0f)).topMax, 70.0f);
}

TEST(ControlRanges, XMaxKeepsTheTopUnderItsLimit)
{
    const auto ranges = ControlRanges::forModel(descriptorWith(100.0f, -60.0f));

    EXPECT_FLOAT_EQ(ranges.clampXMax(150.0f, -60.0f), 120.0f);
    EXPECT_FLOAT_EQ(ranges.clampXMax(200.0f, -120.0f), 180.0f);
}

TEST(ControlRanges, SClipKeepsTheTopUnderItsLimit)
{
    const auto ranges = ControlRanges::forModel(descriptorWith(100.0f, -60.0f));

    EXPECT_FLOAT_EQ(ranges.clampSClip(0.0f, 100.0f), -40.0f);
    EXPECT_FLOAT_EQ(ranges.clampSClip(-200.0f, 100.0f), -120.0f);
}
