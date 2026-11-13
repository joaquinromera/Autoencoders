/// @file LatentPanelPresenter.h
/// @brief Keeps the latent sliders and their strips in step with the state

#pragma once

#include <ILatentPanelPresenter.h>
#include <ILatentPanelView.h>
#include <IMidiAssignments.h>
#include <LatentState.h>
#include <SweepJob.h>
#include <SweepScheduler.h>
#include <TrackAnalysis.h>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

/// @brief Presenter of the latent sliders
///
/// Each slider's histogram depends on the other sliders only, so moving one
/// slider reshapes every strip but its own; its flag (thumb outside the piece)
/// still follows its value. Sweeps are requested on every change and arrive
/// later, once the sliders settle.
class LatentPanelPresenter
    : public ILatentPanelPresenter
    , private LatentState::Listener
{
public:
    LatentPanelPresenter(LatentState &state, ILatentPanelView &view, SweepScheduler &sweeps);
    ~LatentPanelPresenter() override;

    LatentPanelPresenter(const LatentPanelPresenter &)            = delete;
    LatentPanelPresenter &operator=(const LatentPanelPresenter &) = delete;

    /// @param analysis Null when the model has no track: sliders without strips
    void setModel(std::shared_ptr<const TrackAnalysis> analysis, const ControlRanges &ranges);

    /// @brief A sweep finished; ignored unless it is the latest requested
    void onSweepResult(const SweepResult &result);

    /// @brief Shows the MIDI tags; null hides them (map mode off)
    void showCcTags(const IMidiAssignments *assignments);

    /// @brief The user picked a control to learn a controller for
    std::function<void(MidiTarget)> onLearnRequested;

    void onSliderDragged(int index, float value) override;
    void onCcTagClicked(int index) override;

private:
    void latentStateChanged(const LatentState &state) override;
    void refreshHistograms(const std::vector<float> &z);
    void showSweeps();

    LatentState                         &mState;
    ILatentPanelView                    &mView;
    SweepScheduler                      &mSweeps;
    std::shared_ptr<const TrackAnalysis> mAnalysis;
    std::vector<float>                   mShownZ;     ///< z the strips were last computed for
    std::vector<HistogramView>           mHistograms; ///< Last histogram of each slider
    std::optional<SweepResult>           mLastSweep;
};
