project(gameboydebugger)

add_library(${PROJECT_NAME}
        src/debugger.cpp
        src/cpu_debugger.cpp
        src/ppu_debugger.cpp
        src/timer_debugger.cpp
        src/memory_bank_debugger.cpp
        src/cartridge_debugger.cpp)

add_library(gb::debugger ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>)
target_link_libraries(${PROJECT_NAME} PRIVATE
        gb::core
        imgui_sfml
        project_warnings)
target_include_directories(${PROJECT_NAME} 
        PUBLIC ${PROJECT_SOURCE_DIR}/include)
