project(gameboycore VERSION 0.0.1)
configure_file(cmake/version.h.in version.h)

add_library(${PROJECT_NAME}
        src/gameboy.cpp
        src/bus.cpp
        src/cartridge.cpp
        src/apu/apu.cpp
        src/cpu/register8.cpp
        src/cpu/register16.cpp
        src/cpu/cpu.cpp
        src/cpu/alu.cpp
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
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CONAN_LIBS}
        project_warnings)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_BINARY_DIR})

add_subdirectory(test)