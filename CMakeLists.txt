cmake_minimum_required (VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)
project (xweb)

add_subdirectory(base)
add_subdirectory(net)

add_executable(testlog ./test/test_log.cpp)
target_link_libraries(testlog base)

add_executable(testnet ./test/test.cpp)
target_link_libraries(testnet base net)

add_executable(testloop ./test/test_loop.cpp)
target_link_libraries(testloop base net)