cmake_minimum_required(VERSION 3.12.4)
project(gameboy CXX)

option(WITH_DEBUGGER "Enable Gameboy Debugger" OFF)
option(WITH_LIBCXX "Use libc++" OFF)

include(cmake/StandardProjectSettings.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/Doxygen.cmake)
include(cmake/StaticAnalyzers.cmake)
if(MSVC)
    include(cmake/StaticLink.cmake)
endif()

find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(SFML COMPONENTS window graphics system CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
target_compile_definitions(project_options INTERFACE
        DEBUG=$<CONFIG:Debug>
        WITH_DEBUGGER=$<BOOL:${WITH_DEBUGGER}>)

if(WITH_LIBCXX)
    target_compile_options(project_options INTERFACE -stdlib=libc++)
    target_link_options(project_options INTERFACE -lc++)
endif()

add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

enable_sanitizers(project_options)
enable_doxygen()

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
    # This sets a global PCH parameter, each project will build its own PCH, which
    # is a good idea if any #define's change
    target_precompile_headers(project_options INTERFACE
            <array> <vector> <string> <unordered_map> <utility>)
endif()

add_subdirectory(3rdparty/imgui_sfml)
add_subdirectory(3rdparty/parallel-hashmap)
add_subdirectory(gameboy)

if(WITH_DEBUGGER)
    add_subdirectory(debugger)
endif()

add_subdirectory(frontend)

option(ENABLE_TESTING "Enable Tests" OFF)
if(ENABLE_TESTING)
    message("building tests")
    enable_testing()
    add_subdirectory(test)
endif()

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/apple/Info.plist)
endif()
