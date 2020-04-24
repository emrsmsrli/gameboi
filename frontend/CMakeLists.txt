# project gameboy

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/sdl_core.cpp
        src/sdl_audio.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        cxxopts::cxxopts
        spdlog::spdlog
        sfml-graphics
        sfml-window
        sfml-system
        SDL2::SDL2
        imgui::sfml
        gb::core
        $<$<BOOL:${WITH_DEBUGGER}>:gb::debugger>
        project_warnings
        project_options)

target_include_directories(${PROJECT_NAME}
        PRIVATE include)