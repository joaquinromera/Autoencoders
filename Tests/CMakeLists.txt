# Presenters against GMock views
add_executable(PresenterTests
    BandHitTestTests.cpp
    LatentPanelPresenterTests.cpp
    MainPresenterTests.cpp
    MapPresenterTests.cpp
    MeterBallisticsTests.cpp
    OutputPresenterTests.cpp
    SnapshotsPresenterTests.cpp
    SweepSchedulerTests.cpp
    TopBarPresenterTests.cpp
)

target_link_libraries(PresenterTests
    PRIVATE
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        autoencoders_presenters
)

target_include_directories(PresenterTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Mocks
)

# Engine, mailboxes and model file reading, with a GMock decoder
add_executable(AutoencoderTests
    AutoencoderTests.cpp
    BackgroundDecoderTests.cpp
    LatestValueMailboxTests.cpp
    ModelFileReaderTests.cpp
    ModelLoadingTests.cpp
    TestMain.cpp
)

target_link_libraries(AutoencoderTests
    PRIVATE
        GTest::gtest
        GTest::gmock
        autoencoders_dsp
        autoencoders_session
        juce::juce_core
        juce::juce_events
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_gui_basics
)

target_include_directories(AutoencoderTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Mocks
)

target_compile_definitions(AutoencoderTests PRIVATE
    AUTOENCODERS_MODELS_DIR="${CMAKE_SOURCE_DIR}/Extras/models"
    JUCE_MODAL_LOOPS_PERMITTED=1 # the worker test runs the message loop
)

# Session state and ranges; no JUCE needed
add_executable(SessionTests
    ControlRangesTests.cpp
    GlideRampTests.cpp
    LatentStateTests.cpp
    SnapshotBankTests.cpp
    StartingLatentTests.cpp
)

target_link_libraries(SessionTests
    PRIVATE
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        autoencoders_session
)

# Analysis units on a synthetic track
add_executable(AnalysisTests
    CloudDensityTests.cpp
    DecodedLevelTests.cpp
    DecodedTonalityTests.cpp
    LatentHistogramTests.cpp
    LatentProjectionTests.cpp
    LogSpectrumResamplerTests.cpp
    NearnessLabelTests.cpp
    TrackAnalysisTests.cpp
    TrackSliceTests.cpp
)

target_link_libraries(AnalysisTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        autoencoders_analysis
)

target_include_directories(AnalysisTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# The analysis on a real model against the numpy reference; skips without it
add_executable(GoldenModelTests
    GoldenModelTests.cpp
)

target_link_libraries(GoldenModelTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        autoencoders_analysis
        autoencoders_dsp
        autoencoders_model_wrappers
)

target_compile_definitions(GoldenModelTests PRIVATE
    AUTOENCODERS_MODELS_DIR="${CMAKE_SOURCE_DIR}/Extras/models"
)

# MIDI assignments and learn
add_executable(MidiTests
    MidiLearnTests.cpp
    MidiParameterMappingTests.cpp
)

target_link_libraries(MidiTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        autoencoders_midi
)

include(GoogleTest)
gtest_discover_tests(PresenterTests DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(AutoencoderTests DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(SessionTests DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(AnalysisTests DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(GoldenModelTests DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(MidiTests DISCOVERY_MODE PRE_TEST)
