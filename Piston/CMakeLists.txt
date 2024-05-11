add_library(Piston Piston.cpp PistonGraphics.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/IdealGas
)

target_link_libraries(Piston
    PRIVATE
    IdealGas
)
