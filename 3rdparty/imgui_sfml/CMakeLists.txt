cmake_minimum_required(VERSION 3.12.4)
project(imgui_sfml)
	
add_library(${PROJECT_NAME} 
	src/imgui.cpp
	src/imgui_demo.cpp
	src/imgui_draw.cpp
	src/imgui_widgets.cpp
	src/imgui-SFML.cpp)
	
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
