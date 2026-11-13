#include <LatentState.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAre;

namespace
{
ControlRanges twoDimRanges()
{
    ControlRanges ranges;
    ranges.z = {
        { 0.0f, 1.0f},
        {-2.0f, 2.0f}
    };
    return ranges;
}

class RecordingListener : public LatentState::Listener
{
public:
    void latentStateChanged(const LatentState &state) override
    {
        ++calls;
        lastZ = state.latent();
    }

    int                calls = 0;
    std::vector<float> lastZ;
};
} // namespace

class LatentStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        state.reset(twoDimRanges(), {0.5f, 0.0f}, 100.0f, -60.0f);
        state.addListener(&listener);
    }

    LatentState       state;
    RecordingListener listener;
};

TEST_F(LatentStateTest, SetSingleDimKeepsOtherDims)
{
    state.setZ(1, 1.5f);

    EXPECT_THAT(state.latent(), ElementsAre(0.5f, 1.5f));
}

TEST_F(LatentStateTest, SetZClampsToRange)
{
    state.setZ(0, 3.0f);
    state.setZ(1, -9.0f);

    EXPECT_THAT(state.latent(), ElementsAre(1.0f, -2.0f));
}

TEST_F(LatentStateTest, ListenerReceivesWholeVector)
{
    state.setZ(0, 0.25f);

    EXPECT_EQ(listener.calls, 1);
    EXPECT_THAT(listener.lastZ, ElementsAre(0.25f, 0.0f));
}

TEST_F(LatentStateTest, UnchangedValueDoesNotNotify)
{
    state.setZ(0, 0.5f);
    state.setXMax(100.0f);
    state.setSClip(-60.0f);

    EXPECT_EQ(listener.calls, 0);
}

TEST_F(LatentStateTest, SetBandMovesBothWithOneNotification)
{
    state.setBand(80.0f, -50.0f);

    EXPECT_EQ(listener.calls, 1);
    EXPECT_FLOAT_EQ(state.xMax(), 80.0f);
    EXPECT_FLOAT_EQ(state.sClip(), -50.0f);
}

TEST_F(LatentStateTest, TopNeverPassesItsLimit)
{
    state.setXMax(150.0f);
    EXPECT_FLOAT_EQ(state.sClip() + state.xMax(), 60.0f);

    state.setSClip(0.0f);
    EXPECT_FLOAT_EQ(state.sClip() + state.xMax(), 60.0f);
}

TEST_F(LatentStateTest, OutOfRangeDimIsIgnored)
{
    state.setZ(5, 1.0f);

    EXPECT_EQ(listener.calls, 0);
}

TEST_F(LatentStateTest, SetLatentClampsEveryDim)
{
    state.setLatent({-1.0f, 5.0f});

    EXPECT_THAT(state.latent(), ElementsAre(0.0f, 2.0f));
}

TEST_F(LatentStateTest, SetLatentOfWrongSizeThrows)
{
    EXPECT_THROW(state.setLatent({1.0f}), std::invalid_argument);
}

TEST_F(LatentStateTest, ResetAppliesDefaults)
{
    auto ranges = twoDimRanges();
    ranges.xMax = {0.0f, 170.0f};

    state.reset(ranges, {0.1f, 0.2f}, 107.5f, -60.0f);

    EXPECT_THAT(state.latent(), ElementsAre(0.1f, 0.2f));
    EXPECT_FLOAT_EQ(state.xMax(), 107.5f);
    EXPECT_FLOAT_EQ(state.sClip(), -60.0f);
    EXPECT_EQ(listener.calls, 1);
}

TEST_F(LatentStateTest, RemovedListenerHearsNothing)
{
    state.removeListener(&listener);
    state.setZ(0, 0.75f);

    EXPECT_EQ(listener.calls, 0);
}
