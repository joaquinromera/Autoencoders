find_package(frugally-deep CONFIG REQUIRED)

# Look for a repo-local LibTorch install first, but preserve any caller-provided
# CMAKE_PREFIX_PATH/Torch_DIR so external installs still work.
list(PREPEND CMAKE_PREFIX_PATH
    "${CMAKE_SOURCE_DIR}/pytorch-install"
    "${CMAKE_SOURCE_DIR}/pytorch-install/libtorch"
)

find_package(Torch CONFIG QUIET)

if(NOT Torch_FOUND)
    message(FATAL_ERROR
        "Torch not found. Install LibTorch into ${CMAKE_SOURCE_DIR}/pytorch-install "
        "(run ./build_pytorch.sh), or point CMake at an existing install with Torch_DIR."
    )
endif()

add_library(autoencoders_model_wrappers STATIC
    DecoderFactory.cpp
    FdeepDecoder.cpp
    TorchDecoder.cpp
)

target_link_libraries(autoencoders_model_wrappers
    PUBLIC
        autoencoders_model
        frugally-deep::fdeep
    PRIVATE
        autoencoders_strings
        "${TORCH_LIBRARIES}"
        juce::juce_core
)

target_include_directories(autoencoders_model_wrappers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Only TorchDecoder sees the PyTorch headers
set_source_files_properties(TorchDecoder.cpp DecoderFactory.cpp PROPERTIES
    INCLUDE_DIRECTORIES "${TORCH_INCLUDE_DIRS}"
)
