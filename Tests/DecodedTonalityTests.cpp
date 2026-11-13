#include <DecodedTonality.h>

#include <gtest/gtest.h>

#include <vector>

TEST(DecodedTonality, FlatSpectrumIsNotTonal)
{
    const std::vector<float> pred(1'025, 0.5f);

    EXPECT_NEAR(tonality(pred, 100.0f, -60.0f), 0.0, 1e-9);
}

TEST(DecodedTonality, FewPeaksAreTonal)
{
    std::vector<float> pred(1'025, 0.0f);
    for (const std::size_t peak : {50U, 100U, 150U})
    {
        pred[peak] = 1.0f; // 100 dB over the rest
    }

    EXPECT_GT(tonality(pred, 100.0f, -60.0f), 0.99);
}

TEST(DecodedTonality, LevelDoesNotChangeIt)
{
    std::vector<float> pred(1'025, 0.2f);
    pred[40] = 0.6f;
    pred[90] = 0.5f;

    EXPECT_NEAR(tonality(pred, 100.0f, -60.0f), tonality(pred, 100.0f, -20.0f), 1e-9);
}

TEST(DecodedTonality, DcBinIsLeftOut)
{
    std::vector<float> pred(1'025, 0.5f);
    pred[0] = 1.0f;

    EXPECT_NEAR(tonality(pred, 100.0f, -60.0f), 0.0, 1e-9);
}

TEST(DecodedTonality, StrengthStretchesTheUsualSpan)
{
    EXPECT_DOUBLE_EQ(tonalStrength(0.1), 0.0);
    EXPECT_DOUBLE_EQ(tonalStrength(0.525), 0.5);
    EXPECT_DOUBLE_EQ(tonalStrength(0.95), 1.0);
}
