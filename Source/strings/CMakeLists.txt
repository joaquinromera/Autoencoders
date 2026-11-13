add_library(autoencoders_strings INTERFACE)

target_sources(autoencoders_strings INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/StringDefinitions.h
)

target_include_directories(autoencoders_strings INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
