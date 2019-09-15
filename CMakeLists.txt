# Top-level CMake file for global config of 'CREP' project.

cmake_minimum_required (VERSION 3.14.5)

project ("CREP")

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

option(WITH_PROFILING "Enables profiling" OFF)
option(WITH_DEBUG_LOG "Enables logging" OFF)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(BOOST_ROOT "/mnt/c/Program Files/boost/boost_1_70_0")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lstdc++fs")

	if (WITH_PROFILING)
		message("set flags for gprof")
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -pg")
		#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
	endif ()
endif ()

find_package(Boost 1.70 REQUIRED)

if (WITH_DEBUG_LOG)
	add_definitions(-DCREP_DEBUG_LOG_TIME=1)
	add_definitions(-DCREP_DEBUG_LOG_VERTEX_EDGE_SPELLS=1)
	add_definitions(-DCREP_DEBUG_LOG_CLUSTER_DYNAMICS=1)
	add_definitions(-DCREP_DEBUG_LOG_EVENTS=1)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
	find_package(libmongocxx REQUIRED)
	find_package(libbsoncxx REQUIRED)
	include_directories(${LIBMONGOCXX_INCLUDE_DIR})
	include_directories(${LIBBSONCXX_INCLUDE_DIR})
	include_directories("/usr/local/include/mongocxx/v_noabi")
	include_directories("/usr/local/include/bsoncxx/v_noabi")
	include_directories("/usr/local/include/libmongoc-1.0")
	include_directories("/usr/local/include/libbson-1.0")
	include_directories("/usr/local/lib")

	#link_directories("lib/metis-5.1.0")
	#add_subdirectory("lib/metis-5.1.0")

	find_package(MPI REQUIRED)

	link_directories("lib/parmetis-4.0.3")
	add_subdirectory("lib/parmetis-4.0.3")

elseif (MSVC)
	#set(libmongocxx_DIR "C:/mongo-cxx-driver/lib/cmake/libmongocxx-3.4.0")
	#set(libmongocxx_DIR "C:/mongo-cxx-driver/lib/cmake/libbsoncxx-3.4.0")
	##include_directories("C:/mongo-cxx-driver/lib")
	##find_package(libmongocxx REQUIRED PATHS "C:/mongo-cxx-driver/lib/cmake/libmongocxx-3.4.0")
	##find_package(libbsoncxx REQUIRED PATHS "C:/mongo-cxx-driver/lib/cmake/libbsoncxx-3.4.0")
	##include_directories(${LIBMONGOCXX_INCLUDE_DIR})
	##include_directories(${LIBBSONCXX_INCLUDE_DIR})
	##include_directories("C:/mongo-c-driver/lib")
	##include_directories("C:/mongo-c-driver/include/libmongoc-1.0")
	##include_directories("C:/mongo-c-driver/include/libbson-1.0")
	##include_directories("C:/mongo-cxx-driver/include/mongocxx/v_noabi/mongocxx")
	##include_directories("C:/mongo-cxx-driver/include/bsoncxx/v_noabi/bsoncxx")
endif ()


# Include sub-projects
add_subdirectory ("CREP")

# Copyright (c) 2019 Martin Schonger