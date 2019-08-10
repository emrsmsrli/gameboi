cmake_minimum_required(VERSION 3.14)
project(GameBoy)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(GameBoy src/main.cpp)
target_link_libraries(GameBoy ${CONAN_LIBS})