cmake_minimum_required(VERSION 3.10)

project(tutorial)

set (CMAKE_CXX_STANDARD 20)

add_executable(tutorial main.cpp Game.cpp)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(tutorial SDL2)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(tutorial imgui::imgui)


# My libraries

add_subdirectory(Piston)
add_subdirectory(Logger)

target_link_libraries(tutorial Piston)
target_link_libraries(tutorial Logger)

target_include_directories(tutorial
                           PRIVATE Piston
                           PRIVATE Logger
)
