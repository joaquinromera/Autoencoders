/// @file MidiCcDefaultsParser.cpp
/// @brief Implementation of parseMidiCcDefaults

#include <MidiCcDefaultsParser.h>

#include <StringDefinitions.h>

MidiCcDefaults parseMidiCcDefaults(const nlohmann::json &root, std::size_t latentDim)
{
    auto       defaults   = MidiCcDefaults::fallback(latentDim);
    const auto automation = root.find(Strings::JsonFields::Automation);
    if (automation == root.end() || !automation->is_object())
    {
        return defaults;
    }

    const auto sliderCcs = automation->value(Strings::JsonFields::DefaultSliderCc, nlohmann::json::array());
    if (sliderCcs.size() != latentDim)
    {
        return defaults;
    }

    defaults.sliderCcs = sliderCcs.get<std::vector<int>>();
    defaults.xMaxCc    = automation->value(Strings::JsonFields::DefaultXMaxCc, MidiCcDefaults::kXMaxCc);
    defaults.sClipCc   = automation->value(Strings::JsonFields::DefaultSClipCc, MidiCcDefaults::kSClipCc);
    return defaults;
}
