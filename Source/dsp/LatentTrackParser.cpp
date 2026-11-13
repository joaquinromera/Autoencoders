/// @file LatentTrackParser.cpp
/// @brief Implementation of parseLatentTrack

#include <LatentTrackParser.h>

#include <JsonFloat.h>
#include <StringDefinitions.h>

#include <vector>

std::shared_ptr<const LatentTrack> parseLatentTrack(const nlohmann::json &root, std::size_t latentDim)
{
    const auto found = root.find(Strings::JsonFields::ZTrack);
    if (found == root.end() || !found->is_array() || found->empty() || latentDim == 0)
    {
        return nullptr;
    }

    std::vector<float> values;
    values.reserve(found->size() * latentDim);

    for (const auto &frame : *found)
    {
        if (!frame.is_array() || frame.size() != latentDim)
        {
            return nullptr;
        }
        for (const auto &value : frame)
        {
            values.push_back(readJsonFloat(value));
        }
    }

    return std::make_shared<const LatentTrack>(latentDim, std::move(values));
}
