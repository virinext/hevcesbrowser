project(hevcparser_test)


include_directories(
    ../include/
)


find_package(Boost COMPONENTS system unit_test_framework REQUIRED)

set(TEST_SOURCES
  main.cpp
  Params.cpp
  Parsing.cpp
)

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
add_executable(hevcparser_test ${TEST_SOURCES})
target_link_libraries(hevcparser_test hevcparser ${Boost_LIBRARIES})
add_dependencies(hevcparser_test hevcparser)

#add_test(hevc_parser_test ${BUILD_DIR}/hevcparser_test)
add_test(hevc_parser_test /home/viktor/projects/hevcesbrowser/build/hevcparser_test)

enable_testing()
