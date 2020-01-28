cmake_minimum_required(VERSION 3.12.4)
project(gameboy)

# Link this 'library' to use the following warnings
add_library(project_warnings INTERFACE)

if(MSVC)
    target_compile_options(project_warnings INTERFACE /W4)
else()
    option(ENABLE_ASAN "Enable address sanitizer" FALSE)
    if(ENABLE_ASAN)
        target_compile_options(project_options INTERFACE -fsanitize=address)
        target_link_libraries(project_options INTERFACE -fsanitize=address)
    endif()

    target_compile_options(project_warnings
            INTERFACE
            -Wall
            -Wextra # reasonable and standard
            -Wshadow # warn the user if a variable declaration shadows one from a parent context
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                               # non-virtual destructor. This helps catch hard to
                               # track down memory errors
            -Wold-style-cast # warn for c-style casts
            -Wcast-align # warn for potential performance problem casts
            -Wunused # warn on anything being unused
            -Woverloaded-virtual # warn if you overload (not override) a virtual function
            -Wpedantic # warn if non-standard C++ is used
            -Wconversion # warn on type conversions that may lose data
            -Wsign-conversion # warn on sign conversions
            -Wnull-dereference # warn if a null dereference is detected
            -Wdouble-promotion # warn if float is implicit promoted to double
            -Wformat=2) # warn on security issues around functions that format output (ie printf)
            
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(project_warnings
                INTERFACE 
                -Wmisleading-indentation # warn if identation implies blocks where blocks do not exist
                -Wduplicated-cond # warn if if / else chain has duplicated conditions
                -Wduplicated-branches # warn if if / else branches have duplicated code
                -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
                -Wuseless-cast) # warn if you perform a cast to the same type
    endif()
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.14/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_check(REQUIRED)

conan_add_remote(NAME bincrafters URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_add_remote(NAME neargye URL https://api.bintray.com/conan/neargye/conan-packages)
conan_cmake_run(CONANFILE conanfile.txt 
        BASIC_SETUP 
        CMAKE_TARGETS
        BUILD_TYPE ${CMAKE_BUILD_TYPE}
        BUILD missing)

# add debugger dependencies
add_subdirectory(3rdparty/imgui_sfml
	    ${CMAKE_CURRENT_BINARY_DIR}/sfml-build)

add_subdirectory(gameboy
	    ${CMAKE_CURRENT_BINARY_DIR}/gbcore-build)
add_subdirectory(debugger
	    ${CMAKE_CURRENT_BINARY_DIR}/gbdebugger-build)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>)
target_link_libraries(${PROJECT_NAME} PRIVATE
        CONAN_PKG::fmt
        CONAN_PKG::sfml
        CONAN_PKG::magic_enum
        imgui_sfml
        gb::core
        gb::debugger
        project_warnings)

file(GLOB RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/*")
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    get_filename_component(RESOURCE_FILE_NAME ${RESOURCE_FILE} NAME)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${RESOURCE_FILE}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/${RESOURCE_FILE_NAME})
endforeach()

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/apple/Info.plist)
endif()