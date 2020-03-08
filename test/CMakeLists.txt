add_executable(gameboycore_test
        src/main.cpp
        src/test_math.cpp
        src/test_reg8.cpp
        src/test_reg16.cpp
        src/test_run_roms.cpp)

target_link_libraries(gameboycore_test PRIVATE
        gb::core
        CONAN_PKG::gtest
        project_warnings
        project_options)

add_custom_command(TARGET gameboycore_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:gameboycore_test>/res)

add_test(NAME gbcore_test COMMAND $<TARGET_FILE:gameboycore_test>)
