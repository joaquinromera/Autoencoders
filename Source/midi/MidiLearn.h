/// @file MidiLearn.h
/// @brief Binds the next controller the user moves to the control they picked

#pragma once

#include <IMidiAssignments.h>
#include <MidiParameterMapping.h>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>

/// @brief MIDI learn over a MidiParameterMapping
///
/// The user picks a control (message thread); the next controller message
/// (MIDI thread) is bound to it and swallowed, and onLearned is posted back
/// to the message thread.
class MidiLearn : public IMidiAssignments
{
public:
    /// @brief Runs a function on the message thread
    using Post = std::function<void(std::function<void()>)>;

    MidiLearn(MidiParameterMapping &mapping, Post post);

    std::optional<int>        controllerFor(MidiTarget target) const override;
    std::optional<MidiTarget> learning() const override;
    void                      beginLearning(MidiTarget target) override;
    void                      cancelLearning() override;

    /// @brief MIDI thread: binds cc if a control is waiting for one
    /// @return true when the message was used for learning and must not drive anything
    bool offer(int cc);

    /// @brief Message thread: a control was bound to a controller
    std::function<void(MidiTarget, int)> onLearned;

private:
    MidiParameterMapping     &mMapping;
    Post                      mPost;
    mutable std::mutex        mMutex;
    std::optional<MidiTarget> mLearning;
    std::shared_ptr<bool>     mAlive = std::make_shared<bool>(true); ///< Posted calls check it still exists
};
