add_library(IdealGas)

target_sources(IdealGas PRIVATE IdealGas.cpp Gas.cpp Orifice.cpp)

target_include_directories(IdealGas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
