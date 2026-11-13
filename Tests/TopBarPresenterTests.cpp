#include <MockViews.h>
#include <TopBarPresenter.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;

class TopBarPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ModelDescriptor d;
        d.name       = "duhast";
        d.latentDim  = 8;
        d.sampleRate = 22'050.0;
        presenter.setModel(d);
    }

    NiceMock<MockTopBarView> view;
    TopBarPresenter          presenter {view};
};

TEST_F(TopBarPresenterTest, WarnsWhenDeviceRateDiffersFromModel)
{
    EXPECT_CALL(view, showOutputStatus("48 kHz · 512"));
    EXPECT_CALL(view, showRateWarning("model trained at 22.05 kHz"));

    presenter.setAudioStatus(48'000.0, 512);
}

TEST_F(TopBarPresenterTest, NoWarningWhenEqual)
{
    EXPECT_CALL(view, showRateWarning(""));

    presenter.setAudioStatus(22'050.0, 512);
}

TEST_F(TopBarPresenterTest, KilohertzDropsTrailingZeros)
{
    EXPECT_EQ(TopBarPresenter::kilohertz(44'100.0), "44.1 kHz");
    EXPECT_EQ(TopBarPresenter::kilohertz(96'000.0), "96 kHz");
}

TEST_F(TopBarPresenterTest, MidiStatusShowsDeviceAndLastController)
{
    EXPECT_CALL(view, showMidiStatus("IAC Driver Bus 1 · CC 21"));
    presenter.setMidiStatus("IAC Driver Bus 1", 21);

    EXPECT_CALL(view, showMidiStatus("no input"));
    presenter.setMidiStatus("", std::nullopt);
}

TEST_F(TopBarPresenterTest, MuteToggles)
{
    std::vector<bool> changes;
    presenter.onMuteChanged = [&](bool muted) {
        changes.push_back(muted);
    };

    EXPECT_CALL(view, showMuted(true));
    EXPECT_CALL(view, showMuted(false));
    presenter.onMuteToggled();
    presenter.onMuteToggled();

    EXPECT_EQ(changes, (std::vector<bool> {true, false}));
}
