project(gameboydebugger CXX)

add_library(${PROJECT_NAME}
        src/debugger.cpp
        src/apu_debugger.cpp
        src/cpu_debugger.cpp
        src/ppu_debugger.cpp
        src/timer_debugger.cpp
        src/joypad_debugger.cpp
        src/memory_bank_debugger.cpp
        src/cartridge_debugger.cpp)

add_library(gb::debugger ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt
        spdlog::spdlog
        magic_enum::magic_enum
        gb::core
        imgui::sfml
        sfml-graphics
        sfml-window
        project_warnings
        project_options)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)
