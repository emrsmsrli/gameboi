# project gameboy

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/sdl_core.cpp
        src/sdl_audio.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        cxxopts::cxxopts
        fmt::fmt
        spdlog::spdlog
        sfml-system
        sfml-window
        sfml-graphics
        imgui::sfml
        SDL2::SDL2
        gb::core
        $<$<BOOL:${WITH_DEBUGGER}>:gb::debugger>
        project_warnings
        project_options)

target_include_directories(${PROJECT_NAME}
        PRIVATE include)