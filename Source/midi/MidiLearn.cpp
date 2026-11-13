/// @file MidiLearn.cpp
/// @brief Implementation of MidiLearn

#include <MidiLearn.h>

MidiLearn::MidiLearn(MidiParameterMapping &mapping, Post post)
    : mMapping(mapping)
    , mPost(std::move(post))
{
}

std::optional<int> MidiLearn::controllerFor(MidiTarget target) const
{
    return mMapping.controllerFor(target);
}

std::optional<MidiTarget> MidiLearn::learning() const
{
    const std::lock_guard lock(mMutex);
    return mLearning;
}

void MidiLearn::beginLearning(MidiTarget target)
{
    const std::lock_guard lock(mMutex);
    mLearning = target;
}

void MidiLearn::cancelLearning()
{
    const std::lock_guard lock(mMutex);
    mLearning.reset();
}

bool MidiLearn::offer(int cc)
{
    std::optional<MidiTarget> target;
    {
        const std::lock_guard lock(mMutex);
        target = std::exchange(mLearning, std::nullopt);
    }
    if (!target)
    {
        return false;
    }
    mMapping.bind(*target, cc);
    mPost([this, alive = std::weak_ptr<bool>(mAlive), learned = *target, cc] {
        if (!alive.expired() && onLearned)
        {
            onLearned(learned, cc);
        }
    });
    return true;
}
