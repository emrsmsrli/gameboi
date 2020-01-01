project(gameboycore_test)

enable_testing()

add_executable(${PROJECT_NAME} 
        src/main.cpp
        src/math.cpp
        src/TestRegister8.cpp
        src/TestRegister16.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE
        gb::core
        gtest_main
        project_warnings)

add_test(gbcore_test ${PROJECT_NAME})