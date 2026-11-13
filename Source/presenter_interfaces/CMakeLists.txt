# Presenter interfaces - header-only library
# This breaks the circular dependency between presenters and views

add_library(autoencoders_presenter_interfaces INTERFACE)

target_include_directories(autoencoders_presenter_interfaces INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# IMapPresenter names the map layers IMapView defines
target_link_libraries(autoencoders_presenter_interfaces INTERFACE
    autoencoders_view_interfaces
)
