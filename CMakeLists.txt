cmake_minimum_required(VERSION 3.14)
project(GameBoy)

# execute_process(COMMAND
#         conan install
#         ${CMAKE_CURRENT_SOURCE_DIR}
#         --build missing -s
#         build_type=$<CONFIG:Debug>)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_compile_definitions(DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>)

# configure gtest
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)
# end configure gtest

include_directories(include)

# Link this 'library' to use the following warnings
add_library(project_warnings INTERFACE)

if(MSVC)
    target_compile_options(project_warnings INTERFACE /W4)
else()
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
            -Wmisleading-indentation # warn if identation implies blocks where blocks do not exist
            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
            -Wnull-dereference # warn if a null dereference is detected
            -Wuseless-cast # warn if you perform a cast to the same type
            -Wdouble-promotion # warn if float is implicit promoted to double
            -Wformat=2 # warn on security issues around functions that format output (ie printf)
            )
    # todo modify compiler specific warnings
    # https://clang.llvm.org/docs/DiagnosticsReference.html
endif()

include(src/CMakeLists.txt)

add_library(GB SHARED ${GAMEBOY_SRC})
target_include_directories(GB PRIVATE include)
target_compile_features(GB PRIVATE cxx_std_17)
target_link_libraries(GB PRIVATE
        ${CONAN_LIBS}
        project_warnings
        --coverage
        )

add_executable(GameBoy main.cpp)
target_compile_features(GameBoy PRIVATE cxx_std_17)
target_link_libraries(GameBoy PRIVATE
        GB
        ${CONAN_LIBS}
        project_warnings
        --coverage
        )

enable_testing()

include(test/CMakeLists.txt)
add_executable(GameBoy-test ${TEST_SRC})
target_compile_features(GameBoy-test PRIVATE cxx_std_17)
target_link_libraries(GameBoy-test PRIVATE
        GB
        ${CONAN_LIBS}
        project_warnings
        gtest_main
        --coverage
        )
add_test(Tester GameBoy-test)
