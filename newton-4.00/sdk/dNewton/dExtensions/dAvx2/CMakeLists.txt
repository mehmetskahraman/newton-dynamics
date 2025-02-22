# Copyright (c) <2014-2017> <Newton Game Dynamics>
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely.

cmake_minimum_required(VERSION 3.9.0 FATAL_ERROR)

set (projectName "ndSolverAvx2")
message (${projectName})

include_directories(../../../.)
include_directories(../../../dCore)
include_directories(../../../dNewton)
include_directories(../../../dTinyxml)
include_directories(../../../dProfiler)
include_directories(../../../dCollision)
include_directories(../../../dNewton/dJoints)
include_directories(../../../dNewton/dModels)
include_directories(../../../dNewton/dIkSolver)
include_directories(../../../dNewton/dParticles)
include_directories(../../../dNewton/dModels/dVehicle)
include_directories(../../../dNewton/dModels/dCharacter)

file(GLOB CPP_SOURCE *.c *.cpp *.h)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/" FILES ${CPP_SOURCE})

if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /arch:AVX2")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /fp:fast /arch:AVX2")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /fp:fast /arch:AVX2")
	add_library(${projectName} STATIC ${CPP_SOURCE})
endif()

if(MINGW)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mavx2 -mfma  ")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx2 -mfma ")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -mavx2 -mfma")
	add_library(${projectName} STATIC ${CPP_SOURCE})
endif()

if(UNIX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -march=haswell ")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=haswell ")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -march=haswell ")

	add_library(${projectName} SHARED ${CPP_SOURCE})
endif()

if(MSVC OR MINGW)
	target_link_options(${projectName} PUBLIC "/DEBUG") 
endif()

install(TARGETS ${projectName}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin)

install(FILES ${HEADERS} DESTINATION include/${projectName})

