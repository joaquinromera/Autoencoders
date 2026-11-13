#include <CloudDensity.h>

#include <gtest/gtest.h>

#include <cmath>

TEST(CloudDensity, ThinSliceGetsHigherGain)
{
    const SliceWeights thin {
        {0.5, 0.5},
        0.01
    };
    const SliceWeights thick {
        {0.5, 0.5},
        0.5
    };

    EXPECT_GT(CloudDensity::alphas(thin).dots[0], CloudDensity::alphas(thick).dots[0]);
    EXPECT_NEAR(CloudDensity::gain(0.5), 0.16 / std::sqrt(0.5), 1e-12);
}

TEST(CloudDensity, FramesUnderTheFloorAreNotDrawn)
{
    const auto alphas = CloudDensity::alphas({
        {0.01, 0.9, 0.9},
        0.5
    });

    EXPECT_EQ(alphas.dots[0], 0.0f);
    EXPECT_GT(alphas.dots[1], 0.0f);
    ASSERT_EQ(alphas.segments.size(), 2U);
    EXPECT_EQ(alphas.segments[0], 0.0f); // joins a hidden frame
    EXPECT_GT(alphas.segments[1], 0.0f);
}
