/// @file Autoencoder.h
/// @brief Core autoencoder class for audio synthesis using neural networks
/// @author Autoencoder Project
/// @date 2025

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

#include <atomic>
#include <memory>
#include <vector>

#include <IDecoder.h>
#include <LatentInputBuffer.h>
#include <ModelBackend.h>
#include <Parameters.h>
#include <SpectrumTap.h>

/// @brief Real-time spectral synthesis from a decoded latent vector
///
/// Every audio block decodes the latest latent vector into a log-power
/// spectrum, scales it with xMax and sClip, and overlap-adds an inverse FFT
/// with random phases.
///
/// Threads: setLatent, setXMax, setSClip and readLatestSpectrum are called
/// from the message thread; getNextAudioBlock and calculate from the audio
/// thread. They meet only through lock-free mailboxes and atomics.
class Autoencoder
{
public:
    /// @param pDecoder Decoder owned by the audio thread
    /// @param pParams Window length and the initial xMax / sClip
    /// @param backend The runtime pDecoder uses
    Autoencoder(std::unique_ptr<IDecoder> pDecoder, Parameters pParams, ModelBackend backend);

    Autoencoder()                               = delete;
    Autoencoder(const Autoencoder &)            = delete;
    Autoencoder &operator=(const Autoencoder &) = delete;
    Autoencoder(Autoencoder &&)                 = delete;
    Autoencoder &operator=(Autoencoder &&)      = delete;
    ~Autoencoder();

    /// @brief Message thread: the latent vector to decode from the next block on
    /// @throws std::invalid_argument if z does not have getLatentDim() values
    void setLatent(const std::vector<float> &z);

    /// @brief Message thread: target magnitude scaling, reached smoothly
    void setXMax(float newValue);

    /// @brief Message thread: target spectral floor in dB, reached smoothly
    void setSClip(float newValue);

    /// @brief Message thread: silences the output; decoding and the spectrum go on
    void setMuted(bool muted);

    /// @brief Message thread: copies the last block's spectrum if a new one arrived
    bool readLatestSpectrum(SpectrumFrame &out);

    /// @brief Number of latent dimensions the decoder takes
    size_t getLatentDim() const;

    /// @brief Gets the backend type used by this autoencoder
    ModelBackend getBackend() const;

    /// @brief Audio thread: synthesises one block into every channel of bufferToFill
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill);

    /// @brief Audio thread: decodes and adds one window to the overlap-add buffer
    /// @param bufferToFillSize The number of samples the block advances
    void calculate(const int bufferToFillSize);

private:
    void  fillRandomPhaseSpectrum(float xMax, float sClip);
    void  addWindowedFrame();
    float writeBlock(const juce::AudioSourceChannelInfo &bufferToFill);

    std::unique_ptr<IDecoder> mDecoder;
    ModelBackend              mBackend;
    Parameters                mParams;
    const unsigned            mRFFTSize; ///< Spectrum bins the decoder produces

    LatentInputBuffer  mLatentInput;
    SpectrumTap        mSpectrumTap;
    std::vector<float> mPrediction; ///< Last decoded spectrum, reused every block

    std::vector<float> mFFTArray; ///< Array for FFT operations sized to the configured FFT
    const int          mMask;     ///< Bit mask for circular buffer indexing
    int                mIndex;    ///< Current read index in the audio buffer
    int                mIdxProc;  ///< Current processing index
    std::vector<float> mAudio;    ///< Circular audio buffer

    juce::Random   mRandom; ///< Random number generator for phase randomization
    juce::dsp::FFT mFFT;    ///< JUCE FFT processor

    std::atomic<float>               mXMaxTarget;    ///< Written by the message thread
    std::atomic<float>               mSClipTarget;   ///< Written by the message thread
    std::atomic<bool>                mMuted {false}; ///< Written by the message thread
    juce::LinearSmoothedValue<float> mSmoothedXMax;  ///< Audio thread only
    juce::LinearSmoothedValue<float> mSmoothedSClip; ///< Audio thread only
};
