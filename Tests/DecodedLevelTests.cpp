#include <DecodedLevel.h>
#include <IDecoder.h>
#include <PieceSpectra.h>
#include <SoundGrid.h>
#include <SoundSweep.h>

#include <gtest/gtest.h>

#include <cmath>

namespace
{
/// Decodes to one bin holding z[0], and remembers every z it was given.
class FirstDimDecoder : public IDecoder
{
public:
    explicit FirstDimDecoder(std::size_t latentDim)
        : mLatentDim(latentDim)
    {
    }

    std::size_t latentDim() const override
    {
        return mLatentDim;
    }

    std::size_t outputDim() const override
    {
        return 1;
    }

    void decodeInto(const std::vector<float> &z, std::vector<float> &out) const override
    {
        seen.push_back(z);
        out.assign(1, z[0]);
    }

    mutable std::vector<std::vector<float>> seen;

private:
    std::size_t mLatentDim;
};
} // namespace

TEST(DecodedLevel, FlatSpectrumMatchesClosedForm)
{
    const std::vector<float> pred(1'025, 0.5f);

    EXPECT_NEAR(levelDb(pred, 100.0f, -60.0f), (10.0 * std::log10(1'025.0)) + 50.0 - 60.0, 1e-9);
}

TEST(DecodedLevel, BrightnessIsDarkFifteenDbBelowThePiece)
{
    const LevelRange piece {30.0, 45.0};

    EXPECT_DOUBLE_EQ(brightness(45.0, piece), 1.0);
    EXPECT_DOUBLE_EQ(brightness(15.0, piece), 0.0);
    EXPECT_DOUBLE_EQ(brightness(30.0, piece), 0.5);
}

TEST(SoundSweep, Gives32PointsHoldingOtherDims)
{
    const FirstDimDecoder decoder(3);
    const auto            profile = SoundSweep::along(decoder, {0.0f, 7.0f, -2.0f}, 0, {1.0f, 4.0f}, 10.0f, 0.0f);
    const auto           &levels  = profile.levels;

    ASSERT_EQ(levels.size(), 32U);
    ASSERT_EQ(profile.tonality.size(), 32U);
    EXPECT_NEAR(levels.front(), 10.0, 1e-5); // level of one bin = pred · xMax
    EXPECT_NEAR(levels.back(), 40.0, 1e-5);
    for (const auto &z : decoder.seen)
    {
        EXPECT_FLOAT_EQ(z[1], 7.0f);
        EXPECT_FLOAT_EQ(z[2], -2.0f);
    }
}

TEST(PieceSpectra, UsesSecondAndNinetyEighthPercentiles)
{
    std::vector<float> values;
    for (int f = 0; f < 100; ++f)
    {
        values.push_back(static_cast<float>(f));
    }
    const LatentTrack     track(1, values);
    const FirstDimDecoder decoder(1);

    const auto range = PieceSpectra::decode(decoder, track).levels(10.0f, 0.0f); // level = 10 · frame index

    // 600 random frames out of 100 levels: the percentiles land near, not on, 20 and 980.
    EXPECT_GE(range.lo, 0.0);
    EXPECT_LE(range.lo, 50.0);
    EXPECT_GE(range.hi, 950.0);
    EXPECT_LE(range.hi, 990.0);
}

TEST(PieceSpectra, LevelsFollowFloorAndRangeWithoutDecodingAgain)
{
    std::vector<float> values;
    for (int f = 0; f < 100; ++f)
    {
        values.push_back(static_cast<float>(f));
    }
    const LatentTrack     track(1, values);
    const FirstDimDecoder decoder(1);
    const auto            piece   = PieceSpectra::decode(decoder, track);
    const auto            decodes = decoder.seen.size();

    const auto base    = piece.levels(10.0f, 0.0f);
    const auto lowered = piece.levels(10.0f, -30.0f);
    const auto steeper = piece.levels(20.0f, 0.0f);

    EXPECT_EQ(decoder.seen.size(), decodes);
    EXPECT_NEAR(lowered.lo, base.lo - 30.0, 1e-9);
    EXPECT_NEAR(lowered.hi, base.hi - 30.0, 1e-9);
    EXPECT_NEAR(steeper.hi, 2.0 * base.hi, 1e-9);
}

TEST(SoundGrid, DecodesEveryPointInOrderAndPassesThePieceLevelsOn)
{
    const FirstDimDecoder decoder(1);
    const SoundGridJob    job {
        3,
        2,
        {{1.0f}, {2.0f}, {3.0f}, {4.0f}},
        10.0f,
        0.0f
    };

    const auto result = SoundGrid::decode(decoder, job, {30.0, 45.0});

    EXPECT_EQ(result.generation, 3U);
    EXPECT_EQ(result.size, 2U);
    ASSERT_EQ(result.levels.size(), 4U);
    EXPECT_NEAR(result.levels[2], 30.0, 1e-5); // level of one bin = pred · xMax
    EXPECT_EQ(result.tonality.size(), 4U);
    EXPECT_DOUBLE_EQ(result.pieceLevels.hi, 45.0);
}
