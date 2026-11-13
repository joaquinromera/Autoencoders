# Plain model data shared by the engine, the analysis and the session.
# Header-only and free of JUCE so analysis code and tests can use it alone.

add_library(autoencoders_model INTERFACE)

target_include_directories(autoencoders_model INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
