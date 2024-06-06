add_library(Piston)

target_sources(Piston
    PRIVATE
    Piston.cpp
    PistonGraphics.cpp
)

target_include_directories(Piston
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Piston
    PRIVATE
    IdealGas
    SDL2::SDL2
    Solver
)
