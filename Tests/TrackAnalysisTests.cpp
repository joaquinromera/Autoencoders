#include <SyntheticTrack.h>
#include <TrackAnalysis.h>

#include <gtest/gtest.h>

#include <memory>

TEST(TrackAnalysis, SliceIsBuiltOnceAndReused)
{
    const auto          track = std::make_shared<const LatentTrack>(syntheticTrack(500));
    const TrackAnalysis analysis(track, std::vector<SliderMinMax>(4, {-5.0f, 25.0f}));

    const auto &first  = analysis.sliceFor({MapAxes::Space::principal, 0, 1});
    const auto &second = analysis.sliceFor({MapAxes::Space::principal, 0, 1});
    const auto &raw    = analysis.sliceFor({MapAxes::Space::raw, 0, 1});

    EXPECT_EQ(&first, &second);
    EXPECT_NE(&first, &raw);
}

TEST(TrackAnalysis, RejectsRangesOfAnotherSize)
{
    const auto track = std::make_shared<const LatentTrack>(syntheticTrack(100));

    EXPECT_THROW(TrackAnalysis(track, std::vector<SliderMinMax>(3, {0.0f, 1.0f})), std::invalid_argument);
}
