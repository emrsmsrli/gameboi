# project gameboy

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/frontend.cpp
        src/sdl_core.cpp
        src/sdl_audio.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
        nlohmann_json::nlohmann_json
        fmt::fmt
        spdlog::spdlog
        SDL2
        sfml-system
        sfml-window
        sfml-graphics
        imgui::sfml
        gb::core
        $<$<BOOL:${WITH_DEBUGGER}>:gb::debugger>
        project_warnings
        project_options)

target_include_directories(${PROJECT_NAME}
        PRIVATE include)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)
