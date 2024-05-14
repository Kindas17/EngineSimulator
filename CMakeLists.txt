cmake_minimum_required(VERSION 3.10)

project(tutorial)

set (CMAKE_CXX_STANDARD 20)

add_executable(tutorial main.cpp Game.cpp)

find_package(SDL2 REQUIRED)
find_package(imgui REQUIRED)

# My libraries

add_subdirectory(Piston)
add_subdirectory(Logger)
add_subdirectory(IdealGas)

target_link_libraries(tutorial
	PRIVATE
	Piston
	Logger
	IdealGas

	imgui::imgui
	SDL2::SDL2
	SDL2::SDL2main
)
