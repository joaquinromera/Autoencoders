/// @file IMidiAssignments.h
/// @brief Which controller drives which control, and learning new assignments

#pragma once

#include <MidiTarget.h>

#include <optional>

/// @brief What the presenters need to know and do about MIDI assignments
class IMidiAssignments
{
public:
    virtual ~IMidiAssignments() = default;

    /// @brief The controller bound to target, if any
    virtual std::optional<int> controllerFor(MidiTarget target) const = 0;

    /// @brief The target waiting for the next controller moved, if any
    virtual std::optional<MidiTarget> learning() const = 0;

    /// @brief The next controller moved gets bound to target
    virtual void beginLearning(MidiTarget target) = 0;

    virtual void cancelLearning() = 0;
};
