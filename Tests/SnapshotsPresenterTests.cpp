#include <MapPresenter.h>
#include <MockViews.h>
#include <PresenterFixtures.h>
#include <SnapshotsPresenter.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;

class SnapshotsPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        presenter.setModel(model.analysis, model.ranges);
        state.reset(model.ranges, model.frame(10), 100.0f, -60.0f);
    }

    /// Stores frame f in slot, then moves z elsewhere
    void storeFrame(int slot, std::size_t f)
    {
        state.setLatent(model.frame(f));
        presenter.onSlotHeld(slot);
        state.setLatent(model.frame(10));
    }

    SyntheticModel              model;
    LatentState                 state;
    SnapshotBank                bank;
    double                      now = 0.0;
    NiceMock<MockSnapshotsView> view;
    SnapshotsPresenter          presenter {state, bank, view, [this] {
                                      return now;
                                           }};
};

TEST_F(SnapshotsPresenterTest, HoldStoresCurrentZ)
{
    EXPECT_CALL(view, showSlot(_, false, _)).Times(9);
    EXPECT_CALL(view, showSlot(3, true, ::testing::Optional(_)));

    presenter.onSlotHeld(3);

    ASSERT_TRUE(bank.at(3).has_value());
    EXPECT_EQ(*bank.at(3), state.latent());
}

TEST_F(SnapshotsPresenterTest, TapStartsGlideToStoredZ)
{
    storeFrame(3, 500);
    const auto from = state.latent();
    const auto to   = model.frame(500);

    presenter.onSlotTapped(3);
    now += SnapshotsPresenter::kDefaultGlideMs / 2.0;
    presenter.onFrameTick();
    EXPECT_NEAR(state.latent()[0], (from[0] + to[0]) / 2.0f, 1e-4);

    now += SnapshotsPresenter::kDefaultGlideMs;
    presenter.onFrameTick();
    EXPECT_EQ(state.latent(), to);
}

TEST_F(SnapshotsPresenterTest, ZeroGlideJumpsAtOnce)
{
    storeFrame(3, 500);
    presenter.onGlideChanged(0);

    presenter.onSlotTapped(3);

    EXPECT_EQ(state.latent(), model.frame(500));
}

TEST_F(SnapshotsPresenterTest, TapOnEmptySlotKeepsZ)
{
    const auto before = state.latent();

    presenter.onSlotTapped(7);
    now += 1'000.0;
    presenter.onFrameTick();

    EXPECT_EQ(state.latent(), before);
}

TEST_F(SnapshotsPresenterTest, DigitRecallsAndShiftDigitStores)
{
    presenter.onGlideChanged(0);
    state.setLatent(model.frame(700));
    presenter.onDigit(9, true);
    EXPECT_TRUE(bank.at(9).has_value());

    state.setLatent(model.frame(10));
    presenter.onDigit(9, false);
    EXPECT_EQ(state.latent(), model.frame(700));
}

TEST_F(SnapshotsPresenterTest, UserGestureCancelsGlide)
{
    storeFrame(3, 500);
    std::optional<std::vector<float>> target {std::vector<float> {}};
    presenter.onGlideTarget = [&](const std::optional<std::vector<float>> &t) {
        target = t;
    };
    presenter.onSlotTapped(3);
    ASSERT_TRUE(target.has_value());

    state.setZ(2, 1.5f); // a drag or MIDI moves z meanwhile
    now += 1'000.0;
    presenter.onFrameTick();

    EXPECT_FLOAT_EQ(state.latent()[2], 1.5f);
    EXPECT_NE(state.latent(), model.frame(500));
    EXPECT_FALSE(target.has_value());
}

TEST_F(SnapshotsPresenterTest, PinsPublishedToMaps)
{
    NiceMock<MockMapView> mapView;
    MapPresenter          map {state, bank, mapView, 0, [] {
                          return 0.0;
                               }};
    map.setModel(model.analysis, {MapAxes::Space::principal, 0, 1}, model.ranges);

    std::vector<MapPin> pins;
    EXPECT_CALL(mapView, showPins(_)).WillOnce(::testing::SaveArg<0>(&pins));
    presenter.onSlotHeld(5);

    ASSERT_EQ(pins.size(), 1U);
    EXPECT_EQ(pins.front().slot, 5);
}
