find_package(nlohmann_json CONFIG REQUIRED)

add_library(autoencoders_dsp STATIC
    Autoencoder.cpp
    AutoencoderFactory.cpp
    BackgroundDecoder.cpp
    LatentTrackParser.cpp
    MidiCcDefaultsParser.cpp
    ModelFileReader.cpp
    Parameters.cpp
)

target_link_libraries(autoencoders_dsp
    PUBLIC
        autoencoders_analysis
        autoencoders_model
        juce::juce_audio_basics
        juce::juce_events
        juce::juce_dsp
        nlohmann_json::nlohmann_json
    PRIVATE
        autoencoders_strings
        autoencoders_model_wrappers
)

target_include_directories(autoencoders_dsp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
