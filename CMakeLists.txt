cmake_minimum_required(VERSION 3.10)

project(tutorial CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(tutorial main.cpp Game.cpp)

add_compile_options(-Wall -Wextra)

find_package(SDL2 REQUIRED)
find_package(imgui REQUIRED)
find_package(implot REQUIRED)

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
	implot::implot
)
