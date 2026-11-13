# What the views derive from a model's latent track: principal axes, slices
# of the piece, histograms, decoded levels and tonality. Plain C++ and Eigen, no JUCE.

find_package(Eigen3 CONFIG REQUIRED)

add_library(autoencoders_analysis STATIC
    CloudDensity.cpp
    DecodedLevel.cpp
    DecodedTonality.cpp
    LatentHistogram.cpp
    LatentProjection.cpp
    LogSpectrumResampler.cpp
    PieceSpectra.cpp
    SoundGrid.cpp
    SoundSweep.cpp
    TrackAnalysis.cpp
    TrackSlice.cpp
)

target_link_libraries(autoencoders_analysis PUBLIC
    autoencoders_model
    Eigen3::Eigen
)

target_include_directories(autoencoders_analysis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
