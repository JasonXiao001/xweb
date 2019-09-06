cmake_minimum_required (VERSION 3.1)

set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")
project (xweb)

add_subdirectory(base)
add_subdirectory(net)

add_executable(testlog ./test/test_log.cpp)
target_link_libraries(testlog base)

add_executable(testnet ./test/test.cpp)
target_link_libraries(testnet base net)

add_executable(testloop ./test/test_loop.cpp)
target_link_libraries(testloop base net)

add_executable(testacceptor ./test/test_acceptor.cpp)
target_link_libraries(testacceptor base net)

add_executable(testacceptorclient ./test/test_acceptor_client.cpp)
target_link_libraries(testacceptorclient base net)

add_executable(testtcp ./test/test_tcpserver.cpp)
target_link_libraries(testtcp base net)

add_executable(testtcpclient ./test/t_tcpclient.cpp)
target_link_libraries(testtcpclient base net)
