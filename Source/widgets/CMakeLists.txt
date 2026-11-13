# Small controls shared by several views

add_library(autoencoders_widgets STATIC
    CcTag.cpp
)

target_link_libraries(autoencoders_widgets
    PUBLIC
        autoencoders_view_interfaces
    PRIVATE
        autoencoders_strings
        juce::juce_gui_basics
)

target_include_directories(autoencoders_widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
