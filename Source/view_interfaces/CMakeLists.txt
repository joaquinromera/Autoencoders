# View interfaces - header-only, plain data only, so presenters can be tested
# against mocks without JUCE components.

add_library(autoencoders_view_interfaces INTERFACE)

target_include_directories(autoencoders_view_interfaces INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
