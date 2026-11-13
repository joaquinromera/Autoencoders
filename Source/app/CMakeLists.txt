juce_add_gui_app(Autoencoders PRODUCT_NAME "Autoencoders" NEEDS_CURL YES)

# JUCE overrides RUNTIME_OUTPUT_DIRECTORY internally; restore the project-wide
# bin/ location so the app bundle ends up at build/bin/Autoencoders.app.
set_target_properties(Autoencoders PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

target_sources(Autoencoders PRIVATE
    Main.cpp
    MainComponent.cpp
    MainGui.cpp
    MidiInputHandler.cpp
)

target_link_libraries(Autoencoders PRIVATE
    autoencoders_strings
    autoencoders_views
    autoencoders_presenters
    autoencoders_presenter_interfaces
    autoencoders_analysis
    autoencoders_dsp
    autoencoders_midi
    autoencoders_model
    autoencoders_model_wrappers
    autoencoders_session
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_include_directories(Autoencoders PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(Autoencoders PRIVATE
    JUCE_MODAL_LOOPS_PERMITTED=1
)

juce_generate_juce_header(Autoencoders)
