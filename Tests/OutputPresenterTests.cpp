#include <MockViews.h>
#include <OutputPresenter.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::SaveArg;

class OutputPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ModelDescriptor d;
        d.winLength    = 2'048;
        d.sampleRate   = 22'050.0;
        d.defaultXMax  = 100.0f;
        d.defaultSClip = -60.0f;
        d.zRange       = {
            {0.0f, 1.0f}
        };
        presenter.setModel(d);
        state.reset(ControlRanges::forModel(d), {0.5f}, 100.0f, -60.0f);
    }

    LatentState              state;
    NiceMock<MockOutputView> view;
    double                   now = 0.0;
    OutputPresenter          presenter {state, view, [this] {
                                   return now;
                                        }};
};

TEST_F(OutputPresenterTest, SpectrumConvertedWithSessionXMaxSClip)
{
    state.setXMax(80.0f);
    state.setSClip(-50.0f);
    std::vector<float> shown;
    EXPECT_CALL(view, showSpectrum(_)).WillOnce(SaveArg<0>(&shown));

    presenter.onSpectrum(std::vector<float>(1'025, 0.5f), 0.1f);

    ASSERT_EQ(shown.size(), 150U);
    EXPECT_FLOAT_EQ(shown.front(), (0.5f * 80.0f) - 50.0f);
}

TEST_F(OutputPresenterTest, MeterShowsOutputLevelInDb)
{
    EXPECT_CALL(view, showMeter(::testing::FloatNear(-20.0f, 1e-4f), ::testing::FloatNear(-20.0f, 1e-4f)));

    presenter.onSpectrum(std::vector<float>(1'025, 0.0f), 0.1f);
}

// The band starts at floor −60, top +40; the top may reach +60.

TEST_F(OutputPresenterTest, DraggingTheBandMovesBothLines)
{
    presenter.onBandPressed(-10.0f, 1.0f);
    presenter.onBandDragged(0.0f);

    EXPECT_FLOAT_EQ(state.sClip(), -50.0f);
    EXPECT_FLOAT_EQ(state.xMax(), 100.0f); // the top moves with the floor
}

TEST_F(OutputPresenterTest, DraggingALineMovesOnlyThatLine)
{
    presenter.onBandPressed(40.5f, 1.0f);
    presenter.onBandDragged(45.5f);
    EXPECT_FLOAT_EQ(state.xMax(), 105.0f);
    EXPECT_FLOAT_EQ(state.sClip(), -60.0f);

    presenter.onBandPressed(-60.0f, 1.0f);
    presenter.onBandDragged(-70.0f);
    EXPECT_FLOAT_EQ(state.sClip(), -70.0f);
    EXPECT_FLOAT_EQ(state.sClip() + state.xMax(), 45.0f); // the top line stays
}

TEST_F(OutputPresenterTest, FloorStopsAtThePlotsBottomAndTheTopStays)
{
    presenter.onBandPressed(-60.0f, 1.0f);
    presenter.onBandDragged(-200.0f);

    EXPECT_FLOAT_EQ(state.sClip(), -120.0f);
    EXPECT_FLOAT_EQ(state.sClip() + state.xMax(), 40.0f);
}

TEST_F(OutputPresenterTest, FloorStopsAtTheTop)
{
    presenter.onBandPressed(-60.0f, 1.0f);
    presenter.onBandDragged(90.0f);

    EXPECT_FLOAT_EQ(state.sClip(), 0.0f);
    EXPECT_FLOAT_EQ(state.xMax(), 40.0f);
}

TEST_F(OutputPresenterTest, BandStopsWhenTheTopReachesItsLimit)
{
    presenter.onBandPressed(-10.0f, 1.0f);
    presenter.onBandDragged(50.0f);

    EXPECT_FLOAT_EQ(state.sClip() + state.xMax(), 60.0f);
    EXPECT_FLOAT_EQ(state.xMax(), 100.0f);
}

TEST_F(OutputPresenterTest, DragIsMeasuredFromThePress)
{
    // Past the limit and back: the line is under the cursor again.
    presenter.onBandPressed(-60.0f, 1.0f);
    presenter.onBandDragged(-200.0f);
    presenter.onBandDragged(-70.0f);

    EXPECT_FLOAT_EQ(state.sClip(), -70.0f);
}

TEST_F(OutputPresenterTest, PressOutsideTheBandDragsNothing)
{
    presenter.onBandPressed(-90.0f, 1.0f);
    presenter.onBandDragged(-80.0f);

    EXPECT_FLOAT_EQ(state.sClip(), -60.0f);
    EXPECT_FLOAT_EQ(state.xMax(), 100.0f);
}

TEST_F(OutputPresenterTest, BandShowsItsLimits)
{
    OutputBand band;
    EXPECT_CALL(view, showBand(_)).WillRepeatedly(SaveArg<0>(&band));

    state.setXMax(90.0f);

    EXPECT_FLOAT_EQ(band.lowestDb, -120.0f);
    EXPECT_FLOAT_EQ(band.highestDb, 60.0f);
}

TEST_F(OutputPresenterTest, BandMarksModelDefaults)
{
    OutputBand band;
    EXPECT_CALL(view, showBand(_)).WillRepeatedly(SaveArg<0>(&band));

    state.setXMax(90.0f);

    EXPECT_TRUE(band.floorAtDefault);
    EXPECT_FALSE(band.rangeAtDefault);
}

TEST_F(OutputPresenterTest, FrequencyAxisFollowsTheDeviceRate)
{
    std::vector<double> hz;
    EXPECT_CALL(view, showFrequencies(_)).WillOnce(SaveArg<0>(&hz));

    presenter.setDeviceSampleRate(48'000.0);

    ASSERT_FALSE(hz.empty());
    EXPECT_NEAR(hz.back(), 24'000.0, 1e-6);
}
