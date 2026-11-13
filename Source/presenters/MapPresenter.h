/// @file MapPresenter.h
/// @brief Keeps one map in step with the latent state and the snapshots

#pragma once

#include <IMapPresenter.h>
#include <IMapView.h>
#include <IMidiAssignments.h>
#include <LatentState.h>
#include <MapSpace.h>
#include <SnapshotBank.h>
#include <SoundMapLayer.h>
#include <UnseenWatch.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

/// @brief Presenter of one map
///
/// The ring follows z on every change. The cloud and the nearness readout
/// depend only on the coordinates the map does not show, so they are
/// recomputed only when those move: dragging on this map leaves its cloud
/// alone, moving anything else reshapes it. The sound background depends on
/// those coordinates and on xMax and sClip; it is asked for only while shown.
class MapPresenter
    : public IMapPresenter
    , private LatentState::Listener
    , private SnapshotBank::Listener
{
public:
    /// @param mapIndex 0 for map A, 1 for map B: which MIDI axis targets it answers to
    /// @param clock Milliseconds, for debouncing the sound grid requests
    MapPresenter(
        LatentState &state, SnapshotBank &bank, IMapView &view, std::size_t mapIndex,
        SoundMapLayer::Scheduler::Clock clock
    );
    ~MapPresenter() override;

    MapPresenter(const MapPresenter &)            = delete;
    MapPresenter &operator=(const MapPresenter &) = delete;

    /// @brief Plots axes of a newly loaded model; the next state change draws it
    /// @param analysis Null when the model has no track; then axes must be raw
    void setModel(std::shared_ptr<const TrackAnalysis> analysis, MapAxes axes, const ControlRanges &ranges);

    /// @brief Shows where a running glide goes; nothing when no glide runs
    void showGlideTarget(const std::optional<std::vector<float>> &target);

    /// @brief A controller moved one of this map's axes (0 = x, 1 = y) to value 0..1
    void setAxisFromMidi(std::size_t axis, float value);

    /// @brief Shows the axes' MIDI tags; null hides them (map mode off)
    void showCcTags(const IMidiAssignments *assignments);

    /// @brief Where sound grid jobs go; none means the sound background stays empty
    void setSoundSubmitter(SoundMapLayer::Scheduler::Submitter submitter);

    /// @brief Called by the GUI timer: submits a settled sound grid request
    void onFrameTick();

    /// @brief A sound grid finished
    void onSoundGrid(const SoundGridResult &result);

    /// @brief The user picked one of this map's axes to learn a controller for
    std::function<void(MidiTarget)> onLearnRequested;

    void onMapDragged(float x, float y) override;
    void onProjectionSelected(int id) override;
    void onLayerSelected(MapLayer layer) override;
    void onCcTagClicked(int index) override;

private:
    void latentStateChanged(const LatentState &state) override;
    void snapshotsChanged(const SnapshotBank &bank) override;
    void applyAxes(MapAxes axes);
    bool refreshCloudIfUnseenMoved(const std::vector<float> &z);
    void requestSoundIfShown();

    LatentState                         &mState;
    SnapshotBank                        &mBank;
    IMapView                            &mView;
    std::size_t                          mMapIndex;
    std::shared_ptr<const TrackAnalysis> mAnalysis;
    ControlRanges                        mRanges;
    std::vector<MapAxes>                 mChoices; ///< What the Projection menu offers, by id
    std::optional<MapSpace>              mSpace;
    UnseenWatch                          mUnseen; ///< Where the cloud was last computed
    MapLayer                             mLayer = MapLayer::piece;
    SoundMapLayer                        mSound;
    std::pair<float, float>              mLastBand {0.0f, 0.0f}; ///< xMax, sClip the sound was last asked for
};
