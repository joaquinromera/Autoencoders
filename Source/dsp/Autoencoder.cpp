/// @file Autoencoder.cpp
/// @brief Implementation of the Autoencoder class for neural network audio synthesis

#include <Autoencoder.h>
#include <StringDefinitions.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>

namespace
{
constexpr float kMinLog10Power = -12.0f;
constexpr float kMaxLog10Power = 12.0f;

// Smoothing ramp for xMax / sClip, about 50 ms at 48 kHz
constexpr int kSmoothingRampSamples = 2'400;

int getFftOrderForWindowLength(const unsigned int winLength)
{
    if (!std::has_single_bit(winLength))
    {
        throw std::invalid_argument("win_length must be a power of two");
    }

    return static_cast<int>(std::countr_zero(winLength));
}

unsigned int checkedSpectrumSize(const IDecoder &decoder, const unsigned int winLength)
{
    const auto expected = (winLength / 2U) + 1U;
    if (decoder.outputDim() != expected)
    {
        throw std::invalid_argument("Model output size does not match win_length");
    }
    return expected;
}
} // namespace

Autoencoder::Autoencoder(std::unique_ptr<IDecoder> pDecoder, Parameters pParams, ModelBackend backend)
    : mDecoder(std::move(pDecoder))
    , mBackend(backend)
    , mParams(pParams)
    , mRFFTSize(checkedSpectrumSize(*mDecoder, mParams.winLength))
    , mLatentInput(mDecoder->latentDim())
    , mSpectrumTap(mRFFTSize)
    , mPrediction(mRFFTSize, 0.0f)
    , mFFTArray(static_cast<size_t>(mParams.winLength) * 2U, 0.0f)
    , mMask(static_cast<int>(mParams.winLength - 1))
    , mIndex(0)
    , mIdxProc(0)
    , mAudio(mParams.winLength, 0.0f)
    , mRandom {}
    , mFFT {getFftOrderForWindowLength(mParams.winLength)}
    , mXMaxTarget(mParams.xMax)
    , mSClipTarget(mParams.sClip)
{
    DBG(Strings::Debug::Autoencoder::InputDepthPrefix << mDecoder->latentDim());
    DBG(Strings::Debug::Autoencoder::OutputDepthPrefix << mDecoder->outputDim());

    mSmoothedXMax.reset(kSmoothingRampSamples);
    mSmoothedSClip.reset(kSmoothingRampSamples);
    mSmoothedXMax.setCurrentAndTargetValue(mParams.xMax);
    mSmoothedSClip.setCurrentAndTargetValue(mParams.sClip);
}

Autoencoder::~Autoencoder()
{
    DBG(Strings::Debug::Autoencoder::Destroying);
}

void Autoencoder::setLatent(const std::vector<float> &z)
{
    mLatentInput.publish(z);
}

void Autoencoder::setXMax(const float newValue)
{
    mXMaxTarget.store(newValue, std::memory_order_relaxed);
}

void Autoencoder::setSClip(const float newValue)
{
    mSClipTarget.store(newValue, std::memory_order_relaxed);
}

void Autoencoder::setMuted(const bool muted)
{
    mMuted.store(muted, std::memory_order_relaxed);
}

bool Autoencoder::readLatestSpectrum(SpectrumFrame &out)
{
    return mSpectrumTap.readLatest(out);
}

size_t Autoencoder::getLatentDim() const
{
    return mDecoder->latentDim();
}

ModelBackend Autoencoder::getBackend() const
{
    return mBackend;
}

void Autoencoder::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    calculate(bufferToFill.numSamples);
    const float rms = writeBlock(bufferToFill);
    mSpectrumTap.publish(mPrediction, rms);

    mIndex += bufferToFill.numSamples;
    mIndex &= mMask;
}

void Autoencoder::calculate(const int bufferToFillSize)
{
    mDecoder->decodeInto(mLatentInput.current(), mPrediction);

    mSmoothedXMax.setTargetValue(mXMaxTarget.load(std::memory_order_relaxed));
    mSmoothedSClip.setTargetValue(mSClipTarget.load(std::memory_order_relaxed));
    mSmoothedXMax.skip(bufferToFillSize - 1);
    mSmoothedSClip.skip(bufferToFillSize - 1);
    fillRandomPhaseSpectrum(mSmoothedXMax.getNextValue(), mSmoothedSClip.getNextValue());

    addWindowedFrame();
    mIdxProc += bufferToFillSize;
    mIdxProc &= mMask;
}

void Autoencoder::fillRandomPhaseSpectrum(const float xMax, const float sClip)
{
    std::fill(mFFTArray.begin(), mFFTArray.end(), 0.0f);

    for (unsigned int i = 0; i < mRFFTSize; ++i)
    {
        float log10Power = ((mPrediction[i] * xMax) + sClip) / 10.0f;
        log10Power =
            std::isfinite(log10Power) ? std::clamp(log10Power, kMinLog10Power, kMaxLog10Power) : kMinLog10Power;
        const float magnitude  = std::pow(10.0f, log10Power * 0.5f);
        const float phase      = mRandom.nextFloat() * juce::MathConstants<float>::twoPi;
        mFFTArray[2 * i]       = magnitude * std::cos(phase);
        mFFTArray[(2 * i) + 1] = magnitude * std::sin(phase);
    }
}

void Autoencoder::addWindowedFrame()
{
    mFFT.performRealOnlyInverseTransform(mFFTArray.data());

    const auto lastIndex = static_cast<float>(mParams.winLength - 1);
    for (unsigned int i = 0; i < mParams.winLength; ++i)
    {
        const float hann = 0.5F * (1 - std::cos(juce::MathConstants<float>::twoPi * static_cast<float>(i) / lastIndex));
        const int   idx  = (mIdxProc + static_cast<int>(i)) & mMask;
        mAudio[static_cast<size_t>(idx)] += mFFTArray[i] * hann;
    }
}

float Autoencoder::writeBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    float      sumOfSquares = 0.0f;
    const bool muted        = mMuted.load(std::memory_order_relaxed);

    for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        const auto  idx      = static_cast<size_t>((mIndex + sample) & mMask);
        const float clipped  = muted ? 0.0f : std::tanh(mAudio[idx]);
        mAudio[idx]          = 0.0f;
        sumOfSquares        += clipped * clipped;

        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            bufferToFill.buffer->setSample(channel, bufferToFill.startSample + sample, clipped);
        }
    }

    return bufferToFill.numSamples > 0 ? std::sqrt(sumOfSquares / static_cast<float>(bufferToFill.numSamples)) : 0.0f;
}
