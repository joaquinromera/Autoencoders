# The user's current position: latent vector, xMax and sClip, and the ranges
# they live in. Every control writes here; the engine and the views listen.

add_library(autoencoders_session STATIC
    ControlRanges.cpp
    GlideRamp.cpp
    LatentState.cpp
    SnapshotBank.cpp
    StartingLatent.cpp
)

target_link_libraries(autoencoders_session PUBLIC
    autoencoders_model
)

target_include_directories(autoencoders_session PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
