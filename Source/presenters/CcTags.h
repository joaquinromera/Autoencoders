/// @file CcTags.h
/// @brief The MIDI map mode tags of a group of controls

#pragma once

#include <CcTagInfo.h>
#include <IMidiAssignments.h>

#include <vector>

/// @brief Builds the tags a region shows in MIDI map mode
class CcTags
{
public:
    CcTags() = delete;

    /// @param assignments Null when MIDI map mode is off: no tags
    static std::vector<CcTagInfo>
    forTargets(const IMidiAssignments *assignments, const std::vector<MidiTarget> &targets)
    {
        std::vector<CcTagInfo> tags;
        if (assignments == nullptr)
        {
            return tags;
        }
        const auto learning = assignments->learning();
        for (const auto &target : targets)
        {
            if (learning == target)
            {
                tags.push_back({CcTagInfo::State::learning, -1});
            }
            else if (const auto cc = assignments->controllerFor(target))
            {
                tags.push_back({CcTagInfo::State::bound, *cc});
            }
            else
            {
                tags.push_back({CcTagInfo::State::none, -1});
            }
        }
        return tags;
    }
};
