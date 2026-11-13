add_library(autoencoders_views STATIC
    AppLookAndFeel.cpp
    CloudImage.cpp
    LatentPanelView.cpp
    LatentSliderView.cpp
    MapBackground.cpp
    MapHeader.cpp
    MapMarks.cpp
    MapView.cpp
    OutputView.cpp
    SettingsView.cpp
    SnapshotSlotView.cpp
    SnapshotsView.cpp
    SpectrumPlot.cpp
    TopBarView.cpp
)

target_link_libraries(autoencoders_views
    PUBLIC
        autoencoders_widgets
        autoencoders_view_interfaces
        autoencoders_presenter_interfaces
    PRIVATE
        autoencoders_strings
        juce::juce_gui_basics
)

target_include_directories(autoencoders_views PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
