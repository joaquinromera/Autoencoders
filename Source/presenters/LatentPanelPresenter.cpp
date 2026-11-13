/// @file LatentPanelPresenter.cpp
/// @brief Implementation of LatentPanelPresenter

#include <LatentPanelPresenter.h>

#include <CcTags.h>
#include <DecodedLevel.h>
#include <DecodedTonality.h>
#include <StringDefinitions.h>

namespace
{
/// The one dim that differs between a and b, or -1 when none or several do
int onlyChangedDim(const std::vector<float> &a, const std::vector<float> &b)
{
    if (a.size() != b.size())
    {
        return -1;
    }
    int changed = -1;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (a[i] != b[i])
        {
            if (changed >= 0)
            {
                return -1;
            }
            changed = static_cast<int>(i);
        }
    }
    return changed;
}
} // namespace

LatentPanelPresenter::LatentPanelPresenter(LatentState &state, ILatentPanelView &view, SweepScheduler &sweeps)
    : mState(state)
    , mView(view)
    , mSweeps(sweeps)
{
    mState.addListener(this);
}

LatentPanelPresenter::~LatentPanelPresenter()
{
    mState.removeListener(this);
}

void LatentPanelPresenter::setModel(std::shared_ptr<const TrackAnalysis> analysis, const ControlRanges &ranges)
{
    mAnalysis = std::move(analysis);
    mShownZ.clear();
    mHistograms.assign(ranges.z.size(), HistogramView {});
    mLastSweep.reset();

    std::vector<LatentSliderSpec> specs;
    for (std::size_t i = 0; i < ranges.z.size(); ++i)
    {
        specs.push_back({Strings::Ui::LatentAxis + std::to_string(i + 1), ranges.z[i].min, ranges.z[i].max});
    }
    mView.buildSliders(specs);
    if (!mAnalysis)
    {
        mView.showWithoutStrips();
    }
}

void LatentPanelPresenter::onSweepResult(const SweepResult &result)
{
    if (mSweeps.isLatest(result.generation))
    {
        mLastSweep = result;
        showSweeps();
    }
}

void LatentPanelPresenter::onSliderDragged(int index, float value)
{
    mState.setZ(static_cast<std::size_t>(index), value);
}

void LatentPanelPresenter::showCcTags(const IMidiAssignments *assignments)
{
    std::vector<MidiTarget> targets;
    for (std::size_t dim = 0; dim < mHistograms.size(); ++dim)
    {
        targets.push_back(MidiTarget::latentDim(dim));
    }
    mView.showCcTags(CcTags::forTargets(assignments, targets));
}

void LatentPanelPresenter::onCcTagClicked(int index)
{
    if (onLearnRequested && index >= 0 && static_cast<std::size_t>(index) < mHistograms.size())
    {
        onLearnRequested(MidiTarget::latentDim(static_cast<std::size_t>(index)));
    }
}

void LatentPanelPresenter::latentStateChanged(const LatentState &state)
{
    const auto &z = state.latent();
    if (z.size() != mHistograms.size())
    {
        return;
    }
    if (mAnalysis)
    {
        refreshHistograms(z);
        mSweeps.request({0, z, state.xMax(), state.sClip()});
    }
    for (std::size_t i = 0; i < z.size(); ++i)
    {
        const bool flagged = mAnalysis && mAnalysis->histogram(i).isOutside(mHistograms[i], z[i]);
        mView.showValue(static_cast<int>(i), z[i], flagged);
    }
}

void LatentPanelPresenter::refreshHistograms(const std::vector<float> &z)
{
    const int  movedDim  = onlyChangedDim(z, mShownZ);
    const bool unchanged = mShownZ == z;
    const auto s         = mAnalysis->projection().standardize(z);
    for (std::size_t i = 0; i < z.size() && !unchanged; ++i)
    {
        if (static_cast<int>(i) == movedDim)
        {
            continue; // a slider does not reshape its own strip
        }
        mHistograms[i] = mAnalysis->histogram(i).at(s, z[i]);
        mView.showHistogram(
            static_cast<int>(i),
            {mHistograms[i].heights.begin(), mHistograms[i].heights.end()},
            static_cast<float>(mHistograms[i].zeroShare),
            mHistograms[i].empty
        );
    }
    mShownZ = z;
}

void LatentPanelPresenter::showSweeps()
{
    if (!mLastSweep)
    {
        return;
    }
    for (std::size_t i = 0; i < mLastSweep->profiles.size(); ++i)
    {
        const auto        &profile = mLastSweep->profiles[i];
        std::vector<float> lit;
        std::vector<float> tonal;
        for (std::size_t k = 0; k < profile.levels.size(); ++k)
        {
            lit.push_back(static_cast<float>(brightness(profile.levels[k], mLastSweep->pieceLevels)));
            tonal.push_back(static_cast<float>(tonalStrength(profile.tonality[k])));
        }
        mView.showSweep(static_cast<int>(i), lit, tonal);
    }
}
