cmake_minimum_required(VERSION 2.8)

set (BUILD_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/build
)

if(NOT IS_DIRECTORY ${BUILD_DIR})
	file (MAKE_DIRECTORY
		${BUILD_DIR}
	)
endif()

set (EXECUTABLE_OUTPUT_PATH ${BUILD_DIR})
set (LIBRARY_OUTPUT_PATH ${BUILD_DIR})

ADD_DEFINITIONS(-std=c++0x) 

add_subdirectory(hevcparser)
add_subdirectory(utils)
enable_testing()
