# project gameboy

add_executable(${PROJECT_NAME}
        src/main.cpp)

target_compile_definitions(${PROJECT_NAME}
        PRIVATE WITH_DEBUGGER=$<BOOL:${ENABLE_DEBUGGER}>)

target_link_libraries(${PROJECT_NAME} PRIVATE
        CONAN_PKG::fmt
        CONAN_PKG::sfml
        imgui::sfml
        gb::core
        $<$<BOOL:${ENABLE_DEBUGGER}>:gb::debugger>
        project_warnings
        project_options)
