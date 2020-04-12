cmake_minimum_required(VERSION 3.12.4)
project(gameboy CXX)

option(WITH_LIBCXX "Use libc++" OFF)

include(cmake/StandardProjectSettings.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/Doxygen.cmake)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Conan.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
target_compile_definitions(project_options INTERFACE DEBUG=$<CONFIG:Debug>)

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


# end cmake configuration

run_conan()

add_subdirectory(3rdparty/imgui_sfml
        ${CMAKE_CURRENT_BINARY_DIR}/imgui-sfml-build)
add_subdirectory(gameboy
        ${CMAKE_CURRENT_BINARY_DIR}/gb-core-build)

option(ENABLE_DEBUGGER "Enable Gameboy Debugger" OFF)
if(ENABLE_DEBUGGER)
    add_subdirectory(debugger
            ${CMAKE_CURRENT_BINARY_DIR}/gb-debugger-build)
endif()

add_subdirectory(frontend
        ${CMAKE_CURRENT_BINARY_DIR}/frontend-build)

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