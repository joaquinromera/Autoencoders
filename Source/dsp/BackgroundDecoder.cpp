/// @file BackgroundDecoder.cpp
/// @brief Implementation of BackgroundDecoder

#include <BackgroundDecoder.h>

#include <DecoderFactory.h>
#include <SoundSweep.h>

#include <stdexcept>

namespace
{
constexpr int kStopTimeoutMs = 4'000;
}

BackgroundDecoder::BackgroundDecoder(
    DecoderSource source, std::shared_ptr<const LatentTrack> track, std::vector<SliderMinMax> zRange,
    Callbacks callbacks
)
    : juce::Thread("Background decoder")
    , mSource(std::move(source))
    , mTrack(std::move(track))
    , mZRange(std::move(zRange))
    , mCallbacks(std::move(callbacks))
{
    if (mTrack == nullptr)
    {
        throw std::invalid_argument("The background decoder needs the piece's track");
    }
    startThread(juce::Thread::Priority::low);
}

BackgroundDecoder::~BackgroundDecoder()
{
    signalThreadShouldExit();
    mWake.signal();
    stopThread(kStopTimeoutMs);
    cancelPendingUpdate();
}

void BackgroundDecoder::submit(SweepJob job)
{
    {
        const std::lock_guard lock(mMutex);
        mPendingJob = std::move(job);
    }
    mWake.signal();
}

void BackgroundDecoder::run()
{
    std::unique_ptr<IDecoder>   decoder;
    std::optional<PieceSpectra> piece;
    try
    {
        decoder = DecoderFactory::create(mSource);
        piece   = PieceSpectra::decode(*decoder, *mTrack);
    }
    catch (const std::exception &e)
    {
        DBG("[BACKGROUND DECODER] " << e.what());
        return;
    }

    while (!threadShouldExit())
    {
        mWake.wait(-1);
        // One wake can stand for several jobs, so run until none is left.
        while (!threadShouldExit() && (runPendingSweep(*decoder, *piece) || runPendingGrid(*decoder, *piece)))
        {
        }
    }
}

void BackgroundDecoder::submitSoundGrid(std::size_t map, SoundGridJob job)
{
    if (map >= kMaps)
    {
        return;
    }
    {
        const std::lock_guard lock(mMutex);
        mPendingGrids[map] = std::move(job);
    }
    mWake.signal();
}

bool BackgroundDecoder::runPendingSweep(const IDecoder &decoder, const PieceSpectra &piece)
{
    std::optional<SweepJob> job;
    {
        const std::lock_guard lock(mMutex);
        job = std::exchange(mPendingJob, std::nullopt);
    }
    if (!job)
    {
        return false;
    }
    auto                  result = sweepAll(decoder, piece, *job);
    const std::lock_guard lock(mMutex);
    mSweepsReady = std::move(result);
    triggerAsyncUpdate();
    return true;
}

bool BackgroundDecoder::runPendingGrid(const IDecoder &decoder, const PieceSpectra &piece)
{
    for (std::size_t map = 0; map < kMaps; ++map)
    {
        std::optional<SoundGridJob> job;
        {
            const std::lock_guard lock(mMutex);
            job = std::exchange(mPendingGrids[map], std::nullopt);
        }
        if (job)
        {
            auto                  result = SoundGrid::decode(decoder, *job, piece.levels(job->xMax, job->sClip));
            const std::lock_guard lock(mMutex);
            mGridsReady[map] = std::move(result);
            triggerAsyncUpdate();
            return true;
        }
    }
    return false;
}

SweepResult BackgroundDecoder::sweepAll(const IDecoder &decoder, const PieceSpectra &piece, const SweepJob &job) const
{
    SweepResult result {job.generation, {}, piece.levels(job.xMax, job.sClip)};
    for (std::size_t dim = 0; dim < job.z.size() && !threadShouldExit(); ++dim)
    {
        result.profiles.push_back(SoundSweep::along(decoder, job.z, dim, mZRange[dim], job.xMax, job.sClip));
    }
    return result;
}

void BackgroundDecoder::handleAsyncUpdate()
{
    std::optional<SweepResult>                        sweeps;
    std::array<std::optional<SoundGridResult>, kMaps> grids;
    {
        const std::lock_guard lock(mMutex);
        sweeps = std::exchange(mSweepsReady, std::nullopt);
        grids  = std::exchange(mGridsReady, {});
    }
    if (sweeps && mCallbacks.sweeps)
    {
        mCallbacks.sweeps(*sweeps);
    }
    for (std::size_t map = 0; map < kMaps; ++map)
    {
        if (grids[map] && mCallbacks.soundGrids)
        {
            mCallbacks.soundGrids(map, *grids[map]);
        }
    }
}
