#include <MidiParameterMapping.h>

#include <gtest/gtest.h>

namespace
{
const std::vector<SliderMinMax> kRanges {
    { 0.0f, 1.0f},
    {-2.0f, 2.0f},
    { 0.0f, 4.0f}
};
} // namespace

TEST(MidiParameterMapping, DefaultsFromFileReplaceHardcoded)
{
    MidiParameterMapping mapping;
    mapping.applyDefaults(
        {
            {30, 31, 32},
            40,
            41
    },
        kRanges
    );

    EXPECT_EQ(mapping.controllerFor(MidiTarget::latentDim(1)), 31);
    EXPECT_EQ(mapping.controllerFor(MidiTarget::xMaxControl()), 40);
    EXPECT_EQ(mapping.controllerFor(MidiTarget::sClipControl()), 41);
    EXPECT_TRUE(mapping.getTargetsForController(1).empty());

    const auto targets = mapping.getTargetsForController(31);
    ASSERT_EQ(targets.size(), 1U);
    EXPECT_DOUBLE_EQ(targets.front().minimum, -2.0);
    EXPECT_DOUBLE_EQ(targets.front().maximum, 2.0);
}

TEST(MidiParameterMapping, MissingDefaultsFallBackTo1ToN16And17)
{
    MidiParameterMapping mapping;
    mapping.applyDefaults(MidiCcDefaults::fallback(3), kRanges);

    EXPECT_EQ(mapping.controllerFor(MidiTarget::latentDim(0)), 1);
    EXPECT_EQ(mapping.controllerFor(MidiTarget::latentDim(2)), 3);
    EXPECT_EQ(mapping.controllerFor(MidiTarget::xMaxControl()), 16);
    EXPECT_EQ(mapping.controllerFor(MidiTarget::sClipControl()), 17);
}

TEST(MidiParameterMapping, XMaxAndSClipKeepTheReplayScriptRanges)
{
    MidiParameterMapping mapping;

    const auto xMax  = mapping.getTargetsForController(16).at(0);
    const auto sClip = mapping.getTargetsForController(17).at(0);
    EXPECT_DOUBLE_EQ(xMax.maximum, 100.0);
    EXPECT_DOUBLE_EQ(sClip.minimum, -100.0);
}

TEST(MidiParameterMapping, BindMovesControllerFromPreviousTarget)
{
    MidiParameterMapping mapping;
    mapping.applyDefaults(MidiCcDefaults::fallback(3), kRanges);

    mapping.bind(MidiTarget::latentDim(2), 1);

    EXPECT_EQ(mapping.controllerFor(MidiTarget::latentDim(2)), 1);
    EXPECT_FALSE(mapping.controllerFor(MidiTarget::latentDim(0)).has_value());
    EXPECT_EQ(mapping.getTargetsForController(1).size(), 1U);
}

TEST(MidiParameterMapping, MapAxisTargetsResolve)
{
    MidiParameterMapping mapping;

    const auto targets = mapping.getTargetsForController(23);
    ASSERT_EQ(targets.size(), 1U);
    EXPECT_EQ(targets.front().target, MidiTarget::mapAxis(1, 1));

    mapping.applyDefaults(MidiCcDefaults::fallback(3), kRanges);
    EXPECT_EQ(mapping.controllerFor(MidiTarget::mapAxis(0, 0)), 20); // a model load keeps the axes
}
