cmake_minimum_required(VERSION 3.12.4)
project(imgui_sfml)
	
add_library(${PROJECT_NAME} 
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_widgets.cpp
	imgui-SFML.cpp)
	
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
