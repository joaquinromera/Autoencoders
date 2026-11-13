/// @file OutputPresenter.h
/// @brief Keeps the output region in step with the engine and the state

#pragma once

#include <BandDrag.h>
#include <IMidiAssignments.h>
#include <IOutputPresenter.h>
#include <IOutputView.h>
#include <LatentState.h>
#include <LogSpectrumResampler.h>
#include <MeterBallistics.h>
#include <ModelDescriptor.h>

#include <functional>
#include <optional>
#include <span>
#include <vector>

/// @brief Presenter of the output region
///
/// Shows the engine's last decoded spectrum through the session's xMax and
/// sClip, the Floor/Range band, and the output level.
class OutputPresenter
    : public IOutputPresenter
    , private LatentState::Listener
{
public:
    using Clock = std::function<double()>;

    OutputPresenter(LatentState &state, IOutputView &view, Clock clock);
    ~OutputPresenter() override;

    OutputPresenter(const OutputPresenter &)            = delete;
    OutputPresenter &operator=(const OutputPresenter &) = delete;

    void setModel(const ModelDescriptor &descriptor);

    /// @brief The rate the device plays at, which sets the frequency axis
    /// (a model trained at another rate is played faster or slower)
    void setDeviceSampleRate(double sampleRate);

    /// @param bins The decoder's output for the last block, before scaling
    /// @param rms Output level of that block
    void onSpectrum(std::span<const float> bins, float rms);

    /// @brief Shows the MIDI tags; null hides them (map mode off)
    void showCcTags(const IMidiAssignments *assignments);

    /// @brief The user picked a control to learn a controller for
    std::function<void(MidiTarget)> onLearnRequested;

    void onBandPressed(float db, float reachDb) override;
    void onBandDragged(float db) override;
    void onFloorEdited(float floorDb) override;
    void onRangeEdited(float rangeDb) override;
    void onCcTagClicked(int index) override;

private:
    /// @brief The part being dragged and where the drag began
    struct BandGrab
    {
        BandPart   part;
        float      pressedDb;
        BandValues start;
    };

    void latentStateChanged(const LatentState &state) override;

    LatentState                        &mState;
    IOutputView                        &mView;
    std::optional<LogSpectrumResampler> mResampler;
    Clock                               mClock;
    MeterBallistics                     mMeter;
    std::size_t                         mBinCount         = 0;
    double                              mModelSampleRate  = 0.0;
    double                              mDeviceSampleRate = 0.0;
    float                               mDefaultXMax      = 0.0f;
    float                               mDefaultSClip     = 0.0f;
    std::vector<float>                  mPoints;
    std::optional<BandGrab>             mGrab;
};
