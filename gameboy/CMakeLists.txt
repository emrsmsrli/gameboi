project(gameboycore VERSION 0.1.0)
configure_file(cmake/version.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/include/gameboy/version.h)

add_library(${PROJECT_NAME}
        src/gameboy.cpp
        src/bus.cpp
        src/cartridge.cpp
        src/apu/apu.cpp
        src/cpu/register8.cpp
        src/cpu/register16.cpp
        src/cpu/cpu.cpp
        src/cpu/alu.cpp
        src/timer/timer.cpp
        src/link/link.cpp
        src/joypad/joypad.cpp
        src/memory/mmu.cpp
        src/memory/controller/mbc1.cpp
        src/memory/controller/mbc2.cpp
        src/memory/controller/mbc3.cpp
        src/ppu/ppu.cpp
        src/util/data_loader.cpp)

add_library(gb::core ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} 
        PRIVATE DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>)
target_link_libraries(${PROJECT_NAME} PUBLIC
        CONAN_PKG::fmt
        CONAN_PKG::magic_enum
        project_warnings)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)

option(ENABLE_TESTING "Enable testing" FALSE)
if(ENABLE_TESTING)
    add_subdirectory(test)
endif()