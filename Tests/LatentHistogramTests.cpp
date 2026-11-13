#include <LatentHistogram.h>
#include <LatentProjection.h>
#include <SyntheticTrack.h>

#include <gtest/gtest.h>

class LatentHistogramTest : public ::testing::Test
{
protected:
    LatentTrack      track      = syntheticTrack();
    LatentProjection projection = LatentProjection::fromTrack(track);
    SliderMinMax     range {-5.0f, 25.0f};
    LatentHistogram  histogram {projection.standardizedCoords(), track, 0, range};

    std::vector<double> onFrame(std::size_t f) const
    {
        return projection.standardize(track.frame(f));
    }
};

TEST_F(LatentHistogramTest, BinsCoverTheRange)
{
    const auto view = histogram.at(onFrame(3), track.frame(3)[0]);

    ASSERT_EQ(view.heights.size(), LatentHistogram::kBins);
    EXPECT_DOUBLE_EQ(*std::max_element(view.heights.begin(), view.heights.end()), 1.0);
    EXPECT_FALSE(view.empty);
}

TEST_F(LatentHistogramTest, EmptyBelowThreePercentMass)
{
    auto s = onFrame(3);
    s[2]   = 40.0; // dim 2 far from anything in the piece

    EXPECT_TRUE(histogram.at(s, track.frame(3)[0]).empty);
}

TEST_F(LatentHistogramTest, FlaggedWhenThumbOutsideSupport)
{
    const auto s = onFrame(3);

    EXPECT_FALSE(histogram.at(s, track.frame(3)[0]).flagged);
    EXPECT_TRUE(histogram.at(s, range.min).flagged); // nothing of the piece down there
}

TEST_F(LatentHistogramTest, OwnDimensionDoesNotReshapeItself)
{
    auto       s       = onFrame(3);
    const auto before  = histogram.at(s, 0.0f);
    s[0]              += 5.0;
    const auto after   = histogram.at(s, 0.0f);

    EXPECT_EQ(before.heights, after.heights);
    EXPECT_DOUBLE_EQ(before.mass, after.mass);
}

namespace
{
/// The synthetic track with dim 0 through a ReLU, as an encoder's last layer leaves it
LatentTrack reluTrack()
{
    const auto         track = syntheticTrack();
    std::vector<float> values;
    for (std::size_t f = 0; f < track.frames(); ++f)
    {
        const auto frame = track.frame(f);
        values.insert(values.end(), frame.begin(), frame.end());
        values[values.size() - 4] = std::max(0.0f, frame[0] - 2.0f);
    }
    return {4, std::move(values)};
}
} // namespace

class ReluHistogramTest : public ::testing::Test
{
protected:
    LatentTrack      track      = reluTrack();
    LatentProjection projection = LatentProjection::fromTrack(track);
    SliderMinMax     range {-1.0f, 20.0f};
    LatentHistogram  histogram {projection.standardizedCoords(), track, 0, range};

    /// Standing on a frame that has dim 0 at rest, where most neighbours do too
    std::vector<double> onFrameAtZero() const
    {
        std::size_t f = 0;
        while (track.frame(f)[0] != 0.0f)
        {
            ++f;
        }
        return projection.standardize(track.frame(f));
    }
};

TEST_F(ReluHistogramTest, FramesAtZeroAreCountedApart)
{
    const auto view = histogram.at(onFrameAtZero(), 0.0f);

    EXPECT_GT(view.zeroShare, 0.3);
    EXPECT_LT(view.zeroShare, 1.0);
    // The other frames fill the bins on their own scale
    EXPECT_DOUBLE_EQ(*std::max_element(view.heights.begin(), view.heights.end()), 1.0);
}

TEST_F(ReluHistogramTest, ThumbAtZeroIsOnThePiece)
{
    EXPECT_FALSE(histogram.at(onFrameAtZero(), 0.0f).flagged);
}

TEST(LatentHistogram, NoBarsWhenAlmostEveryFrameIsAtZero)
{
    // Dim 0 is 0 in 98 of 100 frames; dim 1 spreads them for the slice.
    std::vector<float> values;
    for (int f = 0; f < 100; ++f)
    {
        values.push_back(f < 98 ? 0.0f : 3.0f);
        values.push_back(static_cast<float>(f % 10));
    }
    const LatentTrack     track(2, values);
    const auto            projection = LatentProjection::fromTrack(track);
    const LatentHistogram histogram(projection.standardizedCoords(), track, 0, {-1.0f, 5.0f});
    const auto            view = histogram.at(projection.standardize(track.frame(0)), 3.0f);

    EXPECT_GT(view.zeroShare, 0.95);
    EXPECT_DOUBLE_EQ(*std::max_element(view.heights.begin(), view.heights.end()), 0.0);
    EXPECT_TRUE(view.flagged); // off 0 there is nothing of the piece
}

TEST_F(LatentHistogramTest, TrackWithoutZerosHasNoZeroShare)
{
    EXPECT_DOUBLE_EQ(histogram.at(onFrame(3), 0.0f).zeroShare, 0.0);
}
