project(gameboydebugger CXX)

add_library(${PROJECT_NAME}
        src/debugger.cpp
        src/apu_debugger.cpp
        src/cpu_debugger.cpp
        src/ppu_debugger.cpp
        src/timer_debugger.cpp
        src/joypad_debugger.cpp
        src/link_debugger.cpp
        src/memory_bank_debugger.cpp
        src/cartridge_debugger.cpp
        src/disassembly_db.cpp
        src/disassembly_view.cpp)

add_library(gb::debugger ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt
        spdlog::spdlog
        gb::core
        sfml-window
        sfml-graphics
        imgui::sfml
        project_warnings
        project_options)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)
