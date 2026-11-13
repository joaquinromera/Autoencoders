/// @file BackgroundDecoder.h
/// @brief Decodes on a worker thread what the GUI needs but the audio thread must not compute

#pragma once

#include <DecoderSource.h>
#include <IDecoder.h>
#include <LatentTrack.h>
#include <PieceSpectra.h>
#include <SliderMinMax.h>
#include <SoundGrid.h>
#include <SweepJob.h>

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include <array>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

/// @brief A worker thread with its own decoder
///
/// On start it builds a decoder from the source and decodes a sample of the
/// piece. Then it runs sweep jobs: only the latest submitted job runs, older
/// unstarted ones are dropped. Each result carries the piece's levels at the
/// job's xMax and sClip, and comes back through the callback on the message
/// thread. Grid jobs for the maps run the same way, one slot per map, after
/// any pending sweep.
class BackgroundDecoder
    : private juce::Thread
    , private juce::AsyncUpdater
{
public:
    static constexpr std::size_t kMaps = 2; ///< Grid slots, one per map

    struct Callbacks
    {
        std::function<void(const SweepResult &)>                      sweeps;
        std::function<void(std::size_t map, const SoundGridResult &)> soundGrids;
    };

    /// @param track The piece; the sweeps are compared with it
    /// @throws std::invalid_argument when track is null
    BackgroundDecoder(
        DecoderSource source, std::shared_ptr<const LatentTrack> track, std::vector<SliderMinMax> zRange,
        Callbacks callbacks
    );
    ~BackgroundDecoder() override;

    BackgroundDecoder(const BackgroundDecoder &)            = delete;
    BackgroundDecoder &operator=(const BackgroundDecoder &) = delete;

    /// @brief Message thread: sweep from job.z, replacing any sweep not started yet
    void submit(SweepJob job);

    /// @brief Message thread: decode a grid for map 0 or 1, replacing that map's grid not started yet
    void submitSoundGrid(std::size_t map, SoundGridJob job);

private:
    void        run() override;
    void        handleAsyncUpdate() override;
    bool        runPendingSweep(const IDecoder &decoder, const PieceSpectra &piece);
    bool        runPendingGrid(const IDecoder &decoder, const PieceSpectra &piece);
    SweepResult sweepAll(const IDecoder &decoder, const PieceSpectra &piece, const SweepJob &job) const;

    const DecoderSource                      mSource;
    const std::shared_ptr<const LatentTrack> mTrack;
    const std::vector<SliderMinMax>          mZRange;
    const Callbacks                          mCallbacks;

    juce::WaitableEvent                               mWake;
    std::mutex                                        mMutex; ///< Guards the slots below
    std::optional<SweepJob>                           mPendingJob;
    std::optional<SweepResult>                        mSweepsReady;
    std::array<std::optional<SoundGridJob>, kMaps>    mPendingGrids;
    std::array<std::optional<SoundGridResult>, kMaps> mGridsReady;
};
