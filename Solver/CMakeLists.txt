add_library(Solver)

target_sources(Solver
	PRIVATE
	Solver.cpp
)

target_include_directories(Solver
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
