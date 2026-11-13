/// @file MapPresenter.cpp
/// @brief Implementation of MapPresenter

#include <MapPresenter.h>

#include <CcTags.h>
#include <CloudDensity.h>
#include <MapLabels.h>
#include <ProjectionChoices.h>

MapPresenter::MapPresenter(
    LatentState &state, SnapshotBank &bank, IMapView &view, std::size_t mapIndex, SoundMapLayer::Scheduler::Clock clock
)
    : mState(state)
    , mBank(bank)
    , mView(view)
    , mMapIndex(mapIndex)
    , mSound(view, std::move(clock))
{
    mState.addListener(this);
    mBank.addListener(this);
}

MapPresenter::~MapPresenter()
{
    mBank.removeListener(this);
    mState.removeListener(this);
}

void MapPresenter::setModel(std::shared_ptr<const TrackAnalysis> analysis, MapAxes axes, const ControlRanges &ranges)
{
    mAnalysis = std::move(analysis);
    mRanges   = ranges;
    mChoices  = ProjectionChoices::forModel(ranges.z.size(), mAnalysis != nullptr);
    if (!mAnalysis)
    {
        mLayer = MapLayer::piece; // the sound needs the worker, which needs a track
    }
    mView.showLayer(mLayer, mAnalysis != nullptr);
    applyAxes(axes);
}

void MapPresenter::applyAxes(MapAxes axes)
{
    mSpace.emplace(mAnalysis, axes, mRanges);
    mUnseen.forget();
    mView.showPlainPad(!mSpace->hasCloud());
    mView.showCaption(mSpace->caption());
    if (mSpace->hasCloud())
    {
        mView.setCloudLayout(mSpace->frameLayout());
    }

    const auto *projection = mAnalysis ? &mAnalysis->projection() : nullptr;
    mView.showProjectionMenu(MapLabels::menu(mChoices, axes, projection));
    mSound.reset();
    mLastBand = {};

    if (mState.latent().size() == mRanges.z.size())
    {
        latentStateChanged(mState);
        snapshotsChanged(mBank);
    }
}

void MapPresenter::showGlideTarget(const std::optional<std::vector<float>> &target)
{
    if (!mSpace || !target || target->size() != mRanges.z.size())
    {
        mView.showGlideTarget(false, {});
        return;
    }
    mView.showGlideTarget(true, mSpace->place(mSpace->coordsOf(*target)).at);
}

void MapPresenter::setAxisFromMidi(std::size_t axis, float value)
{
    if (mSpace && !mState.latent().empty() && mState.latent().size() == mRanges.z.size())
    {
        mState.setLatent(mSpace->latentWithAxisFromController(axis, value, mState.latent()));
    }
}

void MapPresenter::showCcTags(const IMidiAssignments *assignments)
{
    mView.showCcTags(
        CcTags::forTargets(assignments, {MidiTarget::mapAxis(mMapIndex, 0), MidiTarget::mapAxis(mMapIndex, 1)})
    );
}

void MapPresenter::onCcTagClicked(int index)
{
    if (onLearnRequested && (index == 0 || index == 1))
    {
        onLearnRequested(MidiTarget::mapAxis(mMapIndex, static_cast<std::size_t>(index)));
    }
}

void MapPresenter::onProjectionSelected(int id)
{
    if (id >= 0 && static_cast<std::size_t>(id) < mChoices.size())
    {
        applyAxes(mChoices[static_cast<std::size_t>(id)]);
    }
}

void MapPresenter::onLayerSelected(MapLayer layer)
{
    if (layer == MapLayer::sound && !mAnalysis)
    {
        return;
    }
    mLayer = layer;
    mView.showLayer(mLayer, true);
    mSound.reset();
    requestSoundIfShown();
}

void MapPresenter::setSoundSubmitter(SoundMapLayer::Scheduler::Submitter submitter)
{
    mSound.setSubmitter(std::move(submitter));
}

void MapPresenter::onFrameTick()
{
    mSound.tick();
}

void MapPresenter::onSoundGrid(const SoundGridResult &result)
{
    if (mLayer == MapLayer::sound)
    {
        mSound.onResult(result);
    }
}

void MapPresenter::requestSoundIfShown()
{
    const auto &z = mState.latent();
    if (mLayer != MapLayer::sound || !mSpace || z.empty() || z.size() != mRanges.z.size())
    {
        return;
    }
    mLastBand = {mState.xMax(), mState.sClip()};
    mSound.request(*mSpace, z, mState.xMax(), mState.sClip());
}

void MapPresenter::onMapDragged(float x, float y)
{
    if (mSpace && mState.latent().size() == mState.ranges().z.size() && !mState.latent().empty())
    {
        mState.setLatent(mSpace->latentAt({x, y}, mState.latent()));
    }
}

void MapPresenter::latentStateChanged(const LatentState &state)
{
    const auto &z = state.latent();
    if (!mSpace || z.empty())
    {
        return;
    }
    const auto coords    = mSpace->coordsOf(z);
    const auto placement = mSpace->place(coords);
    mView.showRing(placement.at, placement.offMap);
    mView.showPosition(MapLabels::position(mSpace->axes(), coords[mSpace->axes().x], coords[mSpace->axes().y]));
    const bool unseenChanged = refreshCloudIfUnseenMoved(z);
    const bool bandChanged   = mLastBand != std::pair {state.xMax(), state.sClip()};
    if (unseenChanged || bandChanged)
    {
        requestSoundIfShown();
    }
}

void MapPresenter::snapshotsChanged(const SnapshotBank &bank)
{
    if (!mSpace)
    {
        return;
    }
    std::vector<MapPin> pins;
    for (std::size_t slot = 0; slot < SnapshotBank::kSlots; ++slot)
    {
        if (const auto &z = bank.at(slot); z && z->size() == mState.ranges().z.size())
        {
            pins.push_back({static_cast<int>(slot), mSpace->place(mSpace->coordsOf(*z)).at});
        }
    }
    mView.showPins(pins);
}

bool MapPresenter::refreshCloudIfUnseenMoved(const std::vector<float> &z)
{
    if (!mSpace->hasCloud())
    {
        return false;
    }
    const auto sliceCoords = mSpace->sliceCoordsOf(z);
    if (!mUnseen.moved(sliceCoords, mSpace->slice().unseen()))
    {
        return false;
    }
    const auto weights = mSpace->slice().weightsAt(sliceCoords);
    const auto alphas  = CloudDensity::alphas(weights);
    mView.showCloudAlphas(alphas.dots, alphas.segments);
    mView.showNearness(MapLabels::nearness(mSpace->slice().nearness(weights.mean)));
    return true;
}
