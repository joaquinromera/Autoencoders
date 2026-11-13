/// @file SpectrumTap.h
/// @brief Lets the GUI see what the audio thread just synthesised

#pragma once

#include <LatestValueMailbox.h>

#include <cstddef>
#include <vector>

/// @brief One audio block as the GUI sees it
struct SpectrumFrame
{
    std::vector<float> bins;       ///< Decoder output, before xMax/sClip scaling
    float              rms = 0.0f; ///< RMS of the output block
};

/// @brief Audio thread publishes the last block's spectrum, the GUI reads it
class SpectrumTap
{
public:
    explicit SpectrumTap(std::size_t binCount)
        : mMailbox(SpectrumFrame {std::vector<float>(binCount, 0.0f), 0.0f})
        , mScratch {std::vector<float>(binCount, 0.0f), 0.0f}
    {
    }

    /// @brief Audio thread: publishes a block; bins must have binCount values
    void publish(const std::vector<float> &bins, float rms)
    {
        mScratch.bins.assign(bins.begin(), bins.end());
        mScratch.rms = rms;
        mMailbox.write(mScratch);
    }

    /// @brief GUI thread: copies the latest block into out if one arrived
    bool readLatest(SpectrumFrame &out)
    {
        return mMailbox.readLatest(out);
    }

private:
    LatestValueMailbox<SpectrumFrame> mMailbox;
    SpectrumFrame                     mScratch; ///< Owned by the audio thread
};
