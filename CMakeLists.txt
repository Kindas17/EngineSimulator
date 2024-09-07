cmake_minimum_required(VERSION 3.22)

project(tutorial CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(tutorial main.cpp Game.cpp)

add_compile_options(
  -Weverything
  -Wno-c++98-compat
  -Wno-double-promotion
  -Wno-implicit-float-conversion
  -Wno-float-conversion
  -Wno-implicit-int-float-conversion
  -Wno-c++98-compat-pedantic
  -Wno-padded
  -Wno-reorder-ctor
  -fPIC)

find_package(SDL2 REQUIRED)
find_package(imgui REQUIRED)
find_package(implot REQUIRED)

add_subdirectory(Piston)
add_subdirectory(Logger)
add_subdirectory(IdealGas)
add_subdirectory(Solver)

target_link_libraries(
  tutorial
  PRIVATE Piston
          Logger
          IdealGas
          Solver
          Geometry
          imgui::imgui
          SDL2::SDL2
          SDL2::SDL2main
          implot::implot)

add_library(torqueCurve SHARED EngineLib.cpp)
target_link_libraries(
  torqueCurve
  PRIVATE Piston
          Logger
          IdealGas
          Solver
          Geometry
          imgui::imgui
          SDL2::SDL2
          SDL2::SDL2main
          implot::implot)
