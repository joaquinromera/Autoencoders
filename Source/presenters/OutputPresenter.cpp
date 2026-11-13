/// @file OutputPresenter.cpp
/// @brief Implementation of OutputPresenter

#include <OutputPresenter.h>

#include <BandHitTest.h>
#include <CcTags.h>

#include <algorithm>
#include <cmath>

namespace
{
// Field values round to one decimal; closer than that counts as the default
constexpr float kDefaultTolerance = 0.05f;
constexpr float kSilenceDb        = -120.0f;
} // namespace

OutputPresenter::OutputPresenter(LatentState &state, IOutputView &view, Clock clock)
    : mState(state)
    , mView(view)
    , mClock(std::move(clock))
{
    mState.addListener(this);
}

OutputPresenter::~OutputPresenter()
{
    mState.removeListener(this);
}

void OutputPresenter::setModel(const ModelDescriptor &descriptor)
{
    mBinCount        = (descriptor.winLength / 2U) + 1U;
    mModelSampleRate = descriptor.sampleRate;
    mDefaultXMax     = descriptor.defaultXMax;
    mDefaultSClip    = descriptor.defaultSClip;
    setDeviceSampleRate(mDeviceSampleRate);
}

void OutputPresenter::setDeviceSampleRate(double sampleRate)
{
    mDeviceSampleRate = sampleRate;
    if (mBinCount == 0)
    {
        return;
    }
    mResampler.emplace(mBinCount, sampleRate > 0.0 ? sampleRate : mModelSampleRate);
    mView.showFrequencies(mResampler->frequencies());
}

void OutputPresenter::onSpectrum(std::span<const float> bins, float rms)
{
    if (!mResampler)
    {
        return;
    }
    mResampler->resampleInto(bins, mPoints);
    for (auto &point : mPoints)
    {
        point = (point * mState.xMax()) + mState.sClip();
    }
    mView.showSpectrum(mPoints);
    mMeter.push(rms > 0.0f ? std::max(kSilenceDb, 20.0f * std::log10(rms)) : kSilenceDb, mClock());
    mView.showMeter(mMeter.level(), mMeter.peak());
}

void OutputPresenter::onBandPressed(float db, float reachDb)
{
    mGrab.reset();
    const float top = mState.sClip() + mState.xMax();
    if (const auto part = BandHitTest::partAt(db, top, mState.sClip(), reachDb))
    {
        const BandValues start {mState.xMax(), mState.sClip()};
        mGrab = BandGrab {*part, db, start};
    }
}

void OutputPresenter::onBandDragged(float db)
{
    if (mGrab)
    {
        const auto moved = BandDrag::apply(mGrab->part, db - mGrab->pressedDb, mGrab->start, mState.ranges());
        mState.setBand(moved.xMax, moved.sClip);
    }
}

void OutputPresenter::onFloorEdited(float floorDb)
{
    mState.setSClip(floorDb);
}

void OutputPresenter::onRangeEdited(float rangeDb)
{
    mState.setXMax(rangeDb);
}

void OutputPresenter::showCcTags(const IMidiAssignments *assignments)
{
    mView.showCcTags(CcTags::forTargets(assignments, {MidiTarget::sClipControl(), MidiTarget::xMaxControl()}));
}

void OutputPresenter::onCcTagClicked(int index)
{
    if (onLearnRequested && (index == 0 || index == 1))
    {
        onLearnRequested(index == 0 ? MidiTarget::sClipControl() : MidiTarget::xMaxControl());
    }
}

void OutputPresenter::latentStateChanged(const LatentState &state)
{
    mView.showBand(
        {state.sClip(),
         state.xMax(),
         std::abs(state.sClip() - mDefaultSClip) < kDefaultTolerance,
         std::abs(state.xMax() - mDefaultXMax) < kDefaultTolerance,
         state.ranges().sClip.min,
         state.ranges().topMax}
    );
}
