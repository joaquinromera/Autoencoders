#include <CloudDensity.h>
#include <MapPresenter.h>
#include <MockViews.h>
#include <PresenterFixtures.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Field;
using ::testing::NiceMock;
using ::testing::SaveArg;

class MapPresenterTest : public ::testing::Test
{
protected:
    void start(MapAxes axes, bool withTrack = true)
    {
        presenter.setModel(withTrack ? model.analysis : nullptr, axes, model.ranges);
        state.reset(model.ranges, model.frame(10), 100.0f, -60.0f);
    }

    SyntheticModel        model;
    LatentState           state;
    SnapshotBank          bank;
    NiceMock<MockMapView> view;
    double                now = 0.0;
    MapPresenter          presenter {state, bank, view, 0, [this] {
                                return now;
                                     }};
    const MapAxes         principal01 {MapAxes::Space::principal, 0, 1};
};

TEST_F(MapPresenterTest, DragSetsTwoPrincipalCoordinatesAndKeepsTheRest)
{
    start(principal01);
    const auto before = model.analysis->projection().project(state.latent());

    presenter.onMapDragged(0.3f, 0.6f);

    const auto  after = model.analysis->projection().project(state.latent());
    const auto &b0    = model.analysis->projection().bounds(0);
    const auto &b1    = model.analysis->projection().bounds(1);
    EXPECT_NEAR(after[0], b0.lo + (0.3 * (b0.hi - b0.lo)), 1e-3);
    EXPECT_NEAR(after[1], b1.lo + (0.6 * (b1.hi - b1.lo)), 1e-3);
    EXPECT_NEAR(after[2], before[2], 1e-4);
    EXPECT_NEAR(after[3], before[3], 1e-4);
}

TEST_F(MapPresenterTest, DragOnOwnMapDoesNotRedrawCloud)
{
    start(principal01);

    EXPECT_CALL(view, showCloudAlphas(_, _)).Times(0);
    presenter.onMapDragged(0.3f, 0.6f);
}

TEST_F(MapPresenterTest, OtherDimChangeRedrawsCloud)
{
    start(principal01);
    auto z  = state.latent();
    z[2]   += 0.5f;

    EXPECT_CALL(view, showCloudAlphas(_, _)).Times(1);
    state.setLatent(z);
}

TEST_F(MapPresenterTest, RawAxesSetZiZjDirectly)
{
    start({MapAxes::Space::raw, 0, 2});
    const auto before = state.latent();

    presenter.onMapDragged(0.5f, 0.75f);

    EXPECT_FLOAT_EQ(state.latent()[0], 10.0f); // middle of -5..25
    EXPECT_FLOAT_EQ(state.latent()[2], 1.0f);  // three quarters of -2..2
    EXPECT_FLOAT_EQ(state.latent()[1], before[1]);
    EXPECT_FLOAT_EQ(state.latent()[3], before[3]);
}

TEST_F(MapPresenterTest, NoTrackShowsPlainPad)
{
    EXPECT_CALL(view, showPlainPad(true));
    EXPECT_CALL(view, setCloudLayout(_)).Times(0);
    EXPECT_CALL(view, showCloudAlphas(_, _)).Times(0);

    start({MapAxes::Space::raw, 0, 1}, false);
}

TEST_F(MapPresenterTest, NearnessLabelThresholds)
{
    MapNearness shown;
    EXPECT_CALL(view, showNearness(_)).WillRepeatedly(SaveArg<0>(&shown));
    start(principal01);

    auto z = state.latent();
    z[2]   = 2.0f; // the piece's dim 2 has a spread of 0.3: far off
    z[3]   = -2.0f;
    state.setLatent(z);

    EXPECT_EQ(shown.kind, MapNearness::Kind::off);
    EXPECT_EQ(shown.text, "off the piece");
}

TEST_F(MapPresenterTest, RingOffMapIsDashed)
{
    start(principal01);

    EXPECT_CALL(view, showRing(_, true));
    auto z = state.latent();
    z[0]   = 25.0f; // the far end of the range, beyond the piece's bounds
    z[1]   = -5.0f;
    state.setLatent(z);
}

TEST_F(MapPresenterTest, ClampsUnprojectedZToRange)
{
    start(principal01);

    presenter.onMapDragged(1.0f, 0.0f);
    presenter.onMapDragged(0.0f, 1.0f);

    for (std::size_t i = 0; i < state.latent().size(); ++i)
    {
        EXPECT_GE(state.latent()[i], model.ranges.z[i].min);
        EXPECT_LE(state.latent()[i], model.ranges.z[i].max);
    }
}

TEST_F(MapPresenterTest, PinsFollowTheSnapshotBank)
{
    start(principal01);

    EXPECT_CALL(view, showPins(::testing::SizeIs(1)));
    bank.store(4, state.latent());
}

TEST_F(MapPresenterTest, ProjectionMenuListsPrincipalAndRawPairs)
{
    std::vector<ProjectionItem> items;
    EXPECT_CALL(view, showProjectionMenu(_)).WillRepeatedly(SaveArg<0>(&items));

    start(principal01);

    const auto principal = std::count_if(items.begin(), items.end(), [](const auto &i) {
        return i.principal;
    });
    EXPECT_EQ(principal, 6);                 // pairs among the first 4 components
    EXPECT_EQ(items.size() - principal, 6U); // pairs of the 4 raw dims
    EXPECT_TRUE(items.front().selected);     // PC1 × PC2
    EXPECT_EQ(items.back().label, "z3 × z4");
}

TEST_F(MapPresenterTest, RawAxesUseStandardizedSlice)
{
    std::vector<ProjectionItem> items;
    EXPECT_CALL(view, showProjectionMenu(_)).WillRepeatedly(SaveArg<0>(&items));
    start(principal01);
    const auto raw13 = std::find_if(items.begin(), items.end(), [](const auto &i) {
        return i.label == "z1 × z3";
    });
    ASSERT_NE(raw13, items.end());

    std::vector<float> dots;
    EXPECT_CALL(view, showCloudAlphas(_, _)).WillOnce(SaveArg<0>(&dots));
    presenter.onProjectionSelected(raw13->id);

    const auto &slice = model.analysis->sliceFor({MapAxes::Space::raw, 0, 2});
    const auto  expected =
        CloudDensity::alphas(slice.weightsAt(model.analysis->projection().standardize(state.latent())));
    EXPECT_EQ(dots, expected.dots);
}

TEST_F(MapPresenterTest, NoTrackMenuOffersOnlyRawPairs)
{
    std::vector<ProjectionItem> items;
    EXPECT_CALL(view, showProjectionMenu(_)).WillRepeatedly(SaveArg<0>(&items));

    start({MapAxes::Space::raw, 0, 1}, false);

    ASSERT_EQ(items.size(), 6U);
    EXPECT_TRUE(std::none_of(items.begin(), items.end(), [](const auto &i) {
        return i.principal;
    }));
}

TEST_F(MapPresenterTest, GlideTargetIsPlacedOnTheMap)
{
    start(principal01);

    EXPECT_CALL(view, showGlideTarget(true, _));
    presenter.showGlideTarget(model.frame(500));

    EXPECT_CALL(view, showGlideTarget(false, _));
    presenter.showGlideTarget(std::nullopt);
}

TEST_F(MapPresenterTest, AxisCcMapsNormalizedToBounds)
{
    start(principal01);
    const auto &projection = model.analysis->projection();
    const auto  before     = projection.project(state.latent());

    presenter.setAxisFromMidi(0, 0.5f); // a centred stick: the piece's mean
    EXPECT_NEAR(projection.project(state.latent())[0], 0.0, 1e-3);

    presenter.setAxisFromMidi(0, 1.0f);
    EXPECT_NEAR(projection.project(state.latent())[0], projection.bounds(0).hi, 1e-3);

    presenter.setAxisFromMidi(1, 0.0f);
    const auto after = projection.project(state.latent());
    EXPECT_NEAR(after[1], projection.bounds(1).lo, 1e-3);
    EXPECT_NEAR(after[2], before[2], 1e-3); // the unseen components stay
}

class SoundLayerTest : public MapPresenterTest
{
protected:
    void SetUp() override
    {
        presenter.setSoundSubmitter([this](const SoundGridJob &job) {
            jobs.push_back(job);
        });
    }

    void settle()
    {
        now += SoundMapLayer::Scheduler::kDebounceMs;
        presenter.onFrameTick();
    }

    SoundGridResult resultFor(const SoundGridJob &job) const
    {
        const auto cells = job.size * job.size;
        return {
            job.generation,
            job.size,
            std::vector<double>(cells, 45.0),
            std::vector<double>(cells, 0.9),
            {30.0, 45.0}
        };
    }

    std::vector<SoundGridJob> jobs;
    const MapAxes             raw01 {MapAxes::Space::raw, 0, 1};
};

TEST_F(SoundLayerTest, SoundIsOfferedOnlyWithATrack)
{
    EXPECT_CALL(view, showLayer(MapLayer::piece, true));
    start(principal01);

    EXPECT_CALL(view, showLayer(MapLayer::piece, false));
    start(raw01, false);
    presenter.onLayerSelected(MapLayer::sound);
    settle();
    EXPECT_TRUE(jobs.empty());
}

TEST_F(SoundLayerTest, ThePieceLayerAsksForNoGrid)
{
    start(principal01);
    settle();

    EXPECT_TRUE(jobs.empty());
}

TEST_F(SoundLayerTest, EachCellIsWhatADragToItsCentreGives)
{
    start(principal01);
    presenter.onLayerSelected(MapLayer::sound);
    settle();
    ASSERT_EQ(jobs.size(), 1U);
    const auto &job = jobs.front();
    ASSERT_EQ(job.size, SoundMapLayer::kCells);
    ASSERT_EQ(job.points.size(), job.size * job.size);
    EXPECT_FLOAT_EQ(job.xMax, 100.0f);
    EXPECT_FLOAT_EQ(job.sClip, -60.0f);

    // Row 2 from the bottom, column 5
    const float cell = 1.0f / static_cast<float>(job.size);
    presenter.onMapDragged(5.5f * cell, 2.5f * cell);
    const auto &point = job.points[(2 * job.size) + 5];
    for (std::size_t i = 0; i < point.size(); ++i)
    {
        EXPECT_NEAR(point[i], state.latent()[i], 1e-5);
    }
}

TEST_F(SoundLayerTest, DraggingOnTheMapKeepsItsGrid)
{
    start(raw01);
    presenter.onLayerSelected(MapLayer::sound);
    settle();

    presenter.onMapDragged(0.2f, 0.7f);
    settle();

    EXPECT_EQ(jobs.size(), 1U);
}

TEST_F(SoundLayerTest, HiddenDimsOrTheBandAskForANewGrid)
{
    start(raw01);
    presenter.onLayerSelected(MapLayer::sound);
    settle();

    state.setZ(2, 1.0f);
    settle();
    state.setXMax(90.0f);
    settle();

    ASSERT_EQ(jobs.size(), 3U);
    EXPECT_FLOAT_EQ(jobs.back().xMax, 90.0f);
}

TEST_F(SoundLayerTest, ResultIsShownAsBrightnessAndTonalStrength)
{
    start(raw01);
    presenter.onLayerSelected(MapLayer::sound);
    settle();

    std::vector<float> lit;
    std::vector<float> tonal;
    EXPECT_CALL(view, showSoundGrid(SoundMapLayer::kCells, _, _)).WillOnce([&](auto, const auto &b, const auto &t) {
        lit   = b;
        tonal = t;
    });
    presenter.onSoundGrid(resultFor(jobs.front()));

    ASSERT_EQ(lit.size(), SoundMapLayer::kCells * SoundMapLayer::kCells);
    EXPECT_FLOAT_EQ(lit.front(), 1.0f);   // as loud as the piece's loudest
    EXPECT_FLOAT_EQ(tonal.front(), 1.0f); // tonality 0.9 is the top of the scale
}

TEST_F(SoundLayerTest, GridForOldAxesIsNotShown)
{
    start(raw01);
    presenter.onLayerSelected(MapLayer::sound);
    settle();

    EXPECT_CALL(view, showSoundGrid(0, _, _)).Times(AnyNumber());
    EXPECT_CALL(view, showSoundGrid(SoundMapLayer::kCells, _, _)).Times(0);
    presenter.onProjectionSelected(1);
    presenter.onSoundGrid(resultFor(jobs.front()));
}
