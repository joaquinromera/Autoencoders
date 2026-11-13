cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(vcpkg
    GIT_REPOSITORY https://github.com/microsoft/vcpkg/
    GIT_TAG 2025.01.13
)
FetchContent_MakeAvailable(vcpkg)
set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")

project(Autoencoders VERSION 1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")

option(ENABLE_CLANG_TIDY "Enable clang-tidy checks during build" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting (sets build type to Debug)" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

if(ENABLE_COVERAGE)
    set(CMAKE_BUILD_TYPE "Debug")
    set(BUILD_TESTS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --coverage -O0 -fno-inline")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --coverage -O0 -fno-inline")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy"
    PATHS
        /opt/homebrew/bin
        /opt/homebrew/Cellar/llvm/*/bin
        /usr/local/bin
    NO_DEFAULT_PATH
)

if(NOT CLANG_TIDY_EXE)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
endif()

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
else()
    message(WARNING "clang-tidy not found!")
endif()

find_package(JUCE CONFIG REQUIRED)

add_subdirectory(Source/strings)
add_subdirectory(Source/model)
add_subdirectory(Source/analysis)
add_subdirectory(Source/model_wrappers)
add_subdirectory(Source/dsp)
add_subdirectory(Source/session)
add_subdirectory(Source/midi)
add_subdirectory(Source/widgets)
add_subdirectory(Source/presenter_interfaces)
add_subdirectory(Source/view_interfaces)
add_subdirectory(Source/views)
add_subdirectory(Source/presenters)
add_subdirectory(Source/app)

if(BUILD_TESTS)
    message(STATUS "Building tests (BUILD_TESTS=ON)")
    find_package(GTest CONFIG REQUIRED)
    enable_testing()
    add_subdirectory(Tests)
else()
    message(STATUS "Skipping tests (BUILD_TESTS=OFF). Use -DBUILD_TESTS=ON to enable.")
endif()

if(CLANG_TIDY_EXE AND ENABLE_CLANG_TIDY)
    message(STATUS "Enabling clang-tidy for target: Autoencoders")
    set_target_properties(Autoencoders
        PROPERTIES
            CXX_CLANG_TIDY
                "${CLANG_TIDY_EXE};--format-style=file;--header-filter=^${CMAKE_SOURCE_DIR}/Source/.*"
    )
elseif(NOT CLANG_TIDY_EXE)
    message(WARNING "clang-tidy not found!")
endif()

add_custom_target(
    cppcheck_run
    COMMAND cppcheck
        --inline-suppr
        --enable=warning,style,performance,portability
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
        --std=c++20
        --suppress=missingIncludeSystem
        --suppress=preprocessorErrorDirective
        --suppress=unknownMacro
        --checkers-report=${CMAKE_BINARY_DIR}/cppcheck-basic-report.txt
        -DMAC=1
        -DJUCE_MAC=1
        -D__APPLE__=1
        -i${CMAKE_BINARY_DIR}/vcpkg_installed/
        -i${CMAKE_SOURCE_DIR}/pytorch/
    COMMENT "Running basic cppcheck analysis (report: ${CMAKE_BINARY_DIR}/cppcheck-basic-report.txt)"
)

add_custom_target(
    clangformat_run
    COMMAND find ${CMAKE_SOURCE_DIR}/Source -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -style=file -i
    COMMAND find ${CMAKE_SOURCE_DIR}/Tests -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -style=file -i
    COMMENT "Formatting source code with clang-format"
)

if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} # Run tests
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/coverage
        COMMAND gcovr
            --gcov-ignore-errors=all
            --root ${CMAKE_SOURCE_DIR}
            --exclude ".*test.*"
            --exclude ".*build.*"
            --exclude ".*pytorch.*"
            --exclude ".*vcpkg.*"
            --exclude ".*\.rl$$"
            --exclude ".*harfbuzz.*"
            --exclude ".*hb-.*"
            --print-summary
            > ${CMAKE_BINARY_DIR}/coverage/coverage_simple.txt
        COMMENT "Generating coverage report"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_dependencies(coverage PresenterTests AutoencoderTests)
endif()
