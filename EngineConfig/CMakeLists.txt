add_library(EngineConfig)

target_sources(EngineConfig PRIVATE EngineConfig.cpp)

target_include_directories(EngineConfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(EngineConfig PUBLIC nlohmann_json::nlohmann_json)