#include <LatentPanelPresenter.h>
#include <MockViews.h>
#include <PresenterFixtures.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;

class LatentPanelPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        sweeps.setSubmitter([this](const SweepJob &job) {
            submitted.push_back(job);
        });
        presenter.setModel(model.analysis, model.ranges);
        state.reset(model.ranges, model.frame(10), 100.0f, -60.0f);
    }

    void settle()
    {
        now += SweepScheduler::kDebounceMs;
        sweeps.tick();
    }

    SweepResult resultFor(std::uint64_t generation, double level) const
    {
        const SoundProfile profile {std::vector<double>(32, level), std::vector<double>(32, 0.9)};
        return {
            generation,
            std::vector<SoundProfile>(4, profile),
            {30.0, 45.0}
        };
    }

    SyntheticModel                model;
    LatentState                   state;
    double                        now = 0.0;
    SweepScheduler                sweeps {[this] {
        return now;
    }};
    std::vector<SweepJob>         submitted;
    NiceMock<MockLatentPanelView> view;
    LatentPanelPresenter          presenter {state, view, sweeps};
};

TEST_F(LatentPanelPresenterTest, SliderDragPublishesSingleDim)
{
    const auto before = state.latent();

    presenter.onSliderDragged(2, 0.5f);

    EXPECT_FLOAT_EQ(state.latent()[2], 0.5f);
    EXPECT_FLOAT_EQ(state.latent()[0], before[0]);
    EXPECT_FLOAT_EQ(state.latent()[1], before[1]);
    EXPECT_FLOAT_EQ(state.latent()[3], before[3]);
}

TEST_F(LatentPanelPresenterTest, OwnStripsNotRecomputedOnOwnMove)
{
    EXPECT_CALL(view, showHistogram(2, _, _, _)).Times(0);
    EXPECT_CALL(view, showHistogram(::testing::Ne(2), _, _, _)).Times(AnyNumber());

    presenter.onSliderDragged(2, 0.5f);
}

TEST_F(LatentPanelPresenterTest, OtherStripsRecomputedOnMove)
{
    EXPECT_CALL(view, showHistogram(0, _, _, _));
    EXPECT_CALL(view, showHistogram(1, _, _, _));
    EXPECT_CALL(view, showHistogram(3, _, _, _));

    presenter.onSliderDragged(2, 0.5f);
}

TEST_F(LatentPanelPresenterTest, SweepRequestedAfterDebounce)
{
    presenter.onSliderDragged(2, 0.5f);
    now += SweepScheduler::kDebounceMs / 2.0;
    sweeps.tick();
    EXPECT_TRUE(submitted.empty());

    settle();
    ASSERT_EQ(submitted.size(), 1U);
    EXPECT_FLOAT_EQ(submitted.front().z[2], 0.5f);
}

TEST_F(LatentPanelPresenterTest, LatestSweepResultWins)
{
    settle();
    presenter.onSliderDragged(2, 0.5f);
    settle();
    ASSERT_EQ(submitted.size(), 2U);

    EXPECT_CALL(view, showSweep(_, ::testing::Each(1.0f), ::testing::Each(1.0f))).Times(4);
    presenter.onSweepResult(resultFor(submitted.back().generation, 45.0));
}

TEST_F(LatentPanelPresenterTest, StaleGenerationResultIgnored)
{
    settle();
    presenter.onSliderDragged(2, 0.5f);
    settle();

    EXPECT_CALL(view, showSweep(_, _, _)).Times(0);
    presenter.onSweepResult(resultFor(submitted.front().generation, 45.0));
}

TEST_F(LatentPanelPresenterTest, FlaggedThumbShownRed)
{
    EXPECT_CALL(view, showValue(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(view, showValue(0, -5.0f, true));

    presenter.onSliderDragged(0, -5.0f); // the piece never goes below 0 on dim 0
}

TEST_F(LatentPanelPresenterTest, EmptyHistogramShowsDashedTrack)
{
    EXPECT_CALL(view, showHistogram(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(view, showHistogram(0, _, _, true));

    presenter.onSliderDragged(2, 2.0f); // far outside the piece on dim 2
}

TEST(LatentPanelPresenterNoTrack, BuildsSlidersWithoutStrips)
{
    SyntheticModel                model;
    LatentState                   state;
    SweepScheduler                sweeps {[] {
        return 0.0;
    }};
    NiceMock<MockLatentPanelView> view;
    LatentPanelPresenter          presenter {state, view, sweeps};

    EXPECT_CALL(view, buildSliders(::testing::SizeIs(4)));
    EXPECT_CALL(view, showWithoutStrips());
    EXPECT_CALL(view, showHistogram(_, _, _, _)).Times(0);

    presenter.setModel(nullptr, model.ranges);
    state.reset(model.ranges, model.frame(10), 100.0f, -60.0f);
}
