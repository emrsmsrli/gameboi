project(gameboycore VERSION 0.5.1 LANGUAGES CXX)
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
        src/memory/controller/mbc5.cpp
        src/ppu/ppu.cpp
        src/util/data_loader.cpp)

add_library(gb::core ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
        CONAN_PKG::fmt
        CONAN_PKG::spdlog
        CONAN_PKG::magic_enum
        project_warnings
        project_options)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)
