# project gameboy

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/sdl_core.cpp
        src/sdl_audio.cpp)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE WITH_DEBUGGER=$<BOOL:${ENABLE_DEBUGGER}>)

target_link_libraries(${PROJECT_NAME} PRIVATE
        spdlog::spdlog
        CONAN_PKG::sfml
        SDL2::SDL2
        imgui::sfml
        gb::core
        $<$<BOOL:${ENABLE_DEBUGGER}>:gb::debugger>
        project_warnings
        project_options)

target_include_directories(${PROJECT_NAME}
        PRIVATE include)