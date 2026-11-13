/// @file MidiParameterMapping.h
/// @brief Which MIDI controller drives which control, and over what range

#pragma once

#include <MidiCcDefaults.h>
#include <MidiTarget.h>
#include <SliderMinMax.h>

#include <optional>
#include <shared_mutex>
#include <vector>

/// @brief Controller assignments of every control
///
/// Read from the MIDI thread, changed from the message thread (model load)
/// and from the MIDI thread (learning); a shared mutex keeps them apart.
class MidiParameterMapping
{
public:
    /// @brief What one controller drives and the value range it spans
    struct Target
    {
        MidiTarget target;
        double     minimum = 0.0;
        double     maximum = 1.0;
    };

    static constexpr int kFirstMapAxisCc = 20;

    /// @brief xMax on CC 16 over 0..100, sClip on CC 17 over −100..0 (the
    /// ranges send_automation_as_midi.py expects), map axes on CC 20..23
    MidiParameterMapping();

    /// @brief A model's CCs: one per latent dim over its range, xMax and sClip;
    /// map axis assignments are kept
    void applyDefaults(const MidiCcDefaults &defaults, const std::vector<SliderMinMax> &zRange);

    /// @brief Binds cc to target, taking it away from whatever it drove before
    void bind(MidiTarget target, int cc);

    std::optional<int> controllerFor(MidiTarget target) const;

    std::vector<Target> getTargetsForController(int cc) const;

private:
    struct Binding
    {
        MidiTarget         target;
        std::optional<int> cc;
        double             minimum = 0.0;
        double             maximum = 1.0;
    };

    std::vector<Binding>      mBindings;
    mutable std::shared_mutex mMutex;
};
