#include <MainPresenter.h>
#include <MockViews.h>
#include <PresenterFixtures.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;

class MainPresenterTest : public ::testing::Test
{
protected:
    ModelDescriptor descriptor() const
    {
        ModelDescriptor d;
        d.name         = "synthetic";
        d.latentDim    = 4;
        d.winLength    = 2'048;
        d.sampleRate   = 22'050.0;
        d.defaultXMax  = 100.0f;
        d.defaultSClip = -60.0f;
        d.zRange       = model.ranges.z;
        return d;
    }

    SyntheticModel                model;
    LatentState                   state;
    SnapshotBank                  bank;
    NiceMock<MockTopBarView>      topBar;
    NiceMock<MockOutputView>      output;
    NiceMock<MockMapView>         mapA;
    NiceMock<MockMapView>         mapB;
    NiceMock<MockLatentPanelView> latent;
    NiceMock<MockSnapshotsView>   snapshots;
    MainPresenter                 presenter {
        state,
        bank,
        {topBar, output, mapA, mapB, latent, snapshots},
        [] {
            return 0.0;
          }
    };
};

TEST_F(MainPresenterTest, SetModelBuildsAnalysisWhenTrackPresent)
{
    EXPECT_CALL(mapA, showPlainPad(false));
    EXPECT_CALL(mapA, setCloudLayout(::testing::SizeIs(2'000)));

    presenter.setModel(descriptor(), model.track, {});

    EXPECT_NE(presenter.analysis(), nullptr);
    EXPECT_FLOAT_EQ(state.xMax(), 100.0f);
    EXPECT_EQ(state.latent().size(), 4U);
}

TEST_F(MainPresenterTest, SetModelWithoutTrackDisablesClouds)
{
    EXPECT_CALL(mapA, showPlainPad(true));
    EXPECT_CALL(mapB, showPlainPad(true));
    EXPECT_CALL(latent, showWithoutStrips());

    presenter.setModel(descriptor(), nullptr, {});

    EXPECT_EQ(presenter.analysis(), nullptr);
}

TEST_F(MainPresenterTest, SetModelStartsOnTheTrackAndClearsSnapshots)
{
    presenter.setModel(descriptor(), model.track, {});
    bank.store(0, state.latent());

    presenter.setModel(descriptor(), model.track, {});

    EXPECT_FALSE(bank.at(0).has_value());
}

TEST_F(MainPresenterTest, MidiEntryUpdatesStateAndViews)
{
    presenter.setModel(descriptor(), model.track, {});

    EXPECT_CALL(latent, showValue(_, _, _)).Times(::testing::AnyNumber());
    EXPECT_CALL(latent, showValue(2, 1.5f, _));
    EXPECT_CALL(mapB, showRing(_, _));
    presenter.setLatentFromMidi(2, 1.5f);

    EXPECT_FLOAT_EQ(state.latent()[2], 1.5f);
}

TEST_F(MainPresenterTest, TopBarDescribesTheModel)
{
    EXPECT_CALL(topBar, showModel("synthetic", "4 latent dims · frugally-deep"));

    presenter.setModel(descriptor(), model.track, {});
}

namespace
{
/// Every control on CC 7, nothing learning until asked
class FakeAssignments : public IMidiAssignments
{
public:
    std::optional<int> controllerFor(MidiTarget) const override
    {
        return 7;
    }

    std::optional<MidiTarget> learning() const override
    {
        return mLearning;
    }

    void beginLearning(MidiTarget target) override
    {
        mLearning = target;
    }

    void cancelLearning() override
    {
        mLearning.reset();
    }

private:
    std::optional<MidiTarget> mLearning;
};
} // namespace

TEST_F(MainPresenterTest, MidiMapModeShowsTagsOnAllRegions)
{
    FakeAssignments assignments;
    presenter.setModel(descriptor(), model.track, {});
    presenter.setMidiAssignments(&assignments);

    const CcTagInfo cc7 {CcTagInfo::State::bound, 7};
    EXPECT_CALL(topBar, showMidiMapMode(true));
    EXPECT_CALL(output, showCcTags(std::vector<CcTagInfo>(2, cc7)));
    EXPECT_CALL(mapA, showCcTags(std::vector<CcTagInfo>(2, cc7)));
    EXPECT_CALL(mapB, showCcTags(std::vector<CcTagInfo>(2, cc7)));
    EXPECT_CALL(latent, showCcTags(std::vector<CcTagInfo>(4, cc7)));
    presenter.toggleMidiMapMode();
    for (auto *mock : std::initializer_list<void *> {&topBar, &output, &mapA, &mapB, &latent})
    {
        ::testing::Mock::VerifyAndClearExpectations(mock);
    }

    EXPECT_CALL(topBar, showMidiMapMode(false));
    EXPECT_CALL(output, showCcTags(::testing::IsEmpty()));
    EXPECT_CALL(mapA, showCcTags(::testing::IsEmpty()));
    EXPECT_CALL(mapB, showCcTags(::testing::IsEmpty()));
    EXPECT_CALL(latent, showCcTags(::testing::IsEmpty()));
    presenter.toggleMidiMapMode();
}

TEST_F(MainPresenterTest, ClickingATagStartsLearningThatControl)
{
    FakeAssignments assignments;
    presenter.setModel(descriptor(), model.track, {});
    presenter.setMidiAssignments(&assignments);
    presenter.toggleMidiMapMode();

    static_cast<ILatentPanelPresenter &>(presenter.latentPanel()).onCcTagClicked(2);

    EXPECT_EQ(assignments.learning(), MidiTarget::latentDim(2));
}

TEST_F(MainPresenterTest, MapAxisFromMidiMovesThatMap)
{
    presenter.setModel(descriptor(), model.track, {});
    const auto before = state.latent();

    presenter.setMapAxisFromMidi(1, 0, 1.0f);

    EXPECT_NE(state.latent(), before);
}
