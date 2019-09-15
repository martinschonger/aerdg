# CMake file for 'CREP' sub-project.

# Base sources required by every executable
set (PROJECT_SOURCES graph.cpp union_find.cpp comp_graph.cpp cluster_state.cpp utility.cpp logger.cpp)

# Add all executables
##add_executable(CREP "CREP.cpp" "CREP.h" ${PROJECT_SOURCES})
add_executable(crep_test "run_tests.h" "run_tests.cpp" ${PROJECT_SOURCES})
add_executable(gpm "general_purpose_main.h" "general_purpose_main.cpp" ${PROJECT_SOURCES})
add_executable(cocopy "create_offline_copy.h" "create_offline_copy.cpp" ${PROJECT_SOURCES})

if (CMAKE_COMPILER_IS_GNUCXX)
	##target_include_directories(CREP PRIVATE ${MPI_CXX_INCLUDE_PATH})
	##target_compile_options(CREP PRIVATE ${MPI_CXX_COMPILE_FLAGS})
	##target_link_options(CREP PRIVATE ${MPI_CXX_LINK_FLAGS})
	##target_link_libraries(CREP PRIVATE Boost::boost PRIVATE ${LIBMONGOCXX_LIBRARIES} PRIVATE ${LIBBSONCXX_LIBRARIES} PRIVATE ${MPI_CXX_LIBRARIES} PRIVATE "parmetis")
	
	target_include_directories(crep_test PRIVATE ${MPI_CXX_INCLUDE_PATH})
	target_compile_options(crep_test PRIVATE ${MPI_CXX_COMPILE_FLAGS})
	target_link_options(crep_test PRIVATE ${MPI_CXX_LINK_FLAGS})
	target_link_libraries(crep_test PRIVATE Boost::boost PRIVATE ${LIBMONGOCXX_LIBRARIES} PRIVATE ${LIBBSONCXX_LIBRARIES} PRIVATE ${MPI_CXX_LIBRARIES} PRIVATE "parmetis" PRIVATE "stdc++fs")
	target_link_libraries(gpm PRIVATE ${LIBMONGOCXX_LIBRARIES} PRIVATE ${LIBBSONCXX_LIBRARIES} PRIVATE "parmetis")
	target_link_libraries(cocopy PRIVATE ${LIBMONGOCXX_LIBRARIES} PRIVATE ${LIBBSONCXX_LIBRARIES})
else ()
	##target_link_libraries(CREP PRIVATE Boost::boost)
endif ()

# Copyright (c) 2019 Martin Schonger