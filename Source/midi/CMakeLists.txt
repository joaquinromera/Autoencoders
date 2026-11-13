# MIDI controller assignments and MIDI learn; no JUCE, so they can be tested alone.

add_library(autoencoders_midi STATIC
    MidiLearn.cpp
    MidiParameterMapping.cpp
)

target_link_libraries(autoencoders_midi PUBLIC
    autoencoders_model
    autoencoders_session
)

target_include_directories(autoencoders_midi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
