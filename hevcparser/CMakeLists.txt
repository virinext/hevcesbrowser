FIND_PACKAGE(Boost)

include_directories(
	include/
	${Boost_INCLUDE_DIR}
)



set(SOURCES
	src/HevcParser.cpp
	src/Hevc.cpp
	src/HevcParserImpl.cpp
	src/BitstreamReader.cpp
	src/HevcUtils.cpp
)

add_library(hevcparser ${SOURCES}) 


find_package(Boost COMPONENTS system unit_test_framework REQUIRED)

set(TEST_SOURCES
  tests/main.cpp
  tests/Params.cpp
  tests/Parsing.cpp
)

if(UNIX)
	ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
endif()

add_executable(hevcparser_test ${TEST_SOURCES}) 
target_link_libraries(hevcparser_test hevcparser ${Boost_LIBRARIES})
add_dependencies(hevcparser_test hevcparser)

add_test(hevcparser_test ${BUILD_DIR}/hevcparser_test)
enable_testing()
