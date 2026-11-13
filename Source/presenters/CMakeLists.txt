add_library(autoencoders_presenters STATIC
    LatentPanelPresenter.cpp
    MainPresenter.cpp
    MapDragToLatent.cpp
    MapLabels.cpp
    MapPresenter.cpp
    MapSpace.cpp
    OutputPresenter.cpp
    SnapshotsPresenter.cpp
    SoundMapLayer.cpp
    TopBarPresenter.cpp
)

target_link_libraries(autoencoders_presenters
    PUBLIC
        autoencoders_analysis
        autoencoders_model
        autoencoders_presenter_interfaces
        autoencoders_session
        autoencoders_view_interfaces
    PRIVATE
        autoencoders_strings
)

target_include_directories(autoencoders_presenters PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
