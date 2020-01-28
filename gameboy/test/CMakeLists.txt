project(gameboycore_test)

enable_testing()

add_executable(${PROJECT_NAME} 
        src/main.cpp
        src/test_math.cpp
        src/test_reg8.cpp
        src/test_reg16.cpp
        src/test_run_roms.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE
        gb::core
        CONAN_PKG::gtest
        project_warnings)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)

add_test(gbcore_test ${PROJECT_NAME})