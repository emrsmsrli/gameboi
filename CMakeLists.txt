cmake_minimum_required(VERSION 3.12.4)
project(gameboy CXX)

include(cmake/StandardProjectSettings.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/Doxygen.cmake)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Conan.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
target_compile_definitions(project_options INTERFACE DEBUG=$<CONFIG:Debug>)

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

option(ENABLE_TESTING "Enable Tests" OFF)
option(ENABLE_DEBUGGER "Enable Gameboy Debugger" OFF)

# end cmake configuration

run_conan()

add_subdirectory(3rdparty/imgui_sfml
        ${CMAKE_CURRENT_BINARY_DIR}/imgui-sfml-build)
add_subdirectory(gameboy
        ${CMAKE_CURRENT_BINARY_DIR}/gb-core-build)
if(ENABLE_DEBUGGER)
    add_subdirectory(debugger
            ${CMAKE_CURRENT_BINARY_DIR}/gb-debugger-build)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_compile_definitions(${PROJECT_NAME} 
        PRIVATE WITH_DEBUGGER=$<BOOL:${ENABLE_DEBUGGER}>)
target_link_libraries(${PROJECT_NAME} PRIVATE
        CONAN_PKG::fmt
        CONAN_PKG::sfml
        imgui_sfml
        gb::core
        $<$<BOOL:${ENABLE_DEBUGGER}>:gb::debugger>
        project_warnings
        project_options)

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/apple/Info.plist)
endif()