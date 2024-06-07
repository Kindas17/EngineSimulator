add_library(IdealGas)

target_sources(IdealGas PRIVATE IdealGas.cpp)

target_include_directories(IdealGas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(IdealGas PRIVATE Geometry)
