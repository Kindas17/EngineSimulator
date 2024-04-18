cmake_minimum_required(VERSION 3.10)

project(tutorial)

add_executable(tutorial main.cpp Game.cpp)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(tutorial SDL2)
