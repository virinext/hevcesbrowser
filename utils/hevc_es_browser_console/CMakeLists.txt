project(hevcesbrowser_console)

set(SOURCES
	src/main.cpp
	src/HEVCInfoWriter.cpp
	src/HEVCInfoAltWriter.cpp
	../common/ConvToString.cpp
)

find_package(Boost COMPONENTS program_options system REQUIRED)

include_directories(
	${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/../../hevcparser/include/
	${CMAKE_CURRENT_SOURCE_DIR}/../common/
)

set(LIBS
	hevcparser
	${Boost_LIBRARIES}
)

add_executable(hevcesbrowser_console ${SOURCES})
target_link_libraries(hevcesbrowser_console ${LIBS})
add_dependencies(hevcesbrowser_console hevcparser)
