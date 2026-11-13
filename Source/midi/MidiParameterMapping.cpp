/// @file MidiParameterMapping.cpp
/// @brief Implementation of MidiParameterMapping

#include <MidiParameterMapping.h>

#include <algorithm>
#include <mutex>

namespace
{
constexpr double kXMaxCcMin = 0.0, kXMaxCcMax = 100.0;
constexpr double kSClipCcMin = -100.0, kSClipCcMax = 0.0;
constexpr int    kMaps = 2, kAxes = 2;
} // namespace

MidiParameterMapping::MidiParameterMapping()
{
    applyDefaults(MidiCcDefaults::fallback(0), {});
    for (int map = 0; map < kMaps; ++map)
    {
        for (int axis = 0; axis < kAxes; ++axis)
        {
            const auto target = MidiTarget::mapAxis(static_cast<std::size_t>(map), static_cast<std::size_t>(axis));
            mBindings.push_back({target, kFirstMapAxisCc + (map * kAxes) + axis, 0.0, 1.0});
        }
    }
}

void MidiParameterMapping::applyDefaults(const MidiCcDefaults &defaults, const std::vector<SliderMinMax> &zRange)
{
    const std::unique_lock lock(mMutex);
    std::erase_if(mBindings, [](const Binding &b) {
        return b.target.kind != MidiTarget::Kind::mapAxis;
    });
    for (std::size_t dim = 0; dim < zRange.size(); ++dim)
    {
        const auto cc = dim < defaults.sliderCcs.size() ? std::optional<int>(defaults.sliderCcs[dim]) : std::nullopt;
        mBindings.push_back({MidiTarget::latentDim(dim), cc, zRange[dim].min, zRange[dim].max});
    }
    mBindings.push_back({MidiTarget::xMaxControl(), defaults.xMaxCc, kXMaxCcMin, kXMaxCcMax});
    mBindings.push_back({MidiTarget::sClipControl(), defaults.sClipCc, kSClipCcMin, kSClipCcMax});
}

void MidiParameterMapping::bind(MidiTarget target, int cc)
{
    const std::unique_lock lock(mMutex);
    for (auto &binding : mBindings)
    {
        if (binding.target == target)
        {
            binding.cc = cc;
        }
        else if (binding.cc == cc)
        {
            binding.cc.reset();
        }
    }
}

std::optional<int> MidiParameterMapping::controllerFor(MidiTarget target) const
{
    const std::shared_lock lock(mMutex);
    const auto             found = std::find_if(mBindings.begin(), mBindings.end(), [&](const Binding &b) {
        return b.target == target;
    });
    return found != mBindings.end() ? found->cc : std::nullopt;
}

std::vector<MidiParameterMapping::Target> MidiParameterMapping::getTargetsForController(int cc) const
{
    const std::shared_lock lock(mMutex);
    std::vector<Target>    targets;
    for (const auto &binding : mBindings)
    {
        if (binding.cc == cc)
        {
            targets.push_back({binding.target, binding.minimum, binding.maximum});
        }
    }
    return targets;
}
