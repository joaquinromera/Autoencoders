#include <gtest/gtest.h>

#include <juce_gui_basics/juce_gui_basics.h>

int main(int argc, char **argv)
{
    // Initialize JUCE GUI subsystem - this creates MessageManager and other singletons
    // ScopedJuceInitialiser_GUI will clean them up when it goes out of scope
    juce::ScopedJuceInitialiser_GUI juceInit;

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    return RUN_ALL_TESTS();
}
