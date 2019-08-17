cmake_minimum_required (VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)
project (xweb)

add_subdirectory(base)

add_executable(testlog ./test/test_log.cpp)
target_link_libraries(testlog base)
