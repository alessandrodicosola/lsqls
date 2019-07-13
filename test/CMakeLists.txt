cmake_minimum_required(VERSION 3.0)

project(lsqls_test)
get_filename_component(PARENT_DIRECTORY ../ ABSOLUTE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
message(STATUS "bin directory for test: ${CMAKE_CURRENT_BINARY_DIR}")

set(EXTERNAL_SOURCES ${PARENT_DIRECTORY}/src/file64.cpp)

set(TEST1 test_path)
add_test(NAME ${TEST1} COMMAND ${TEST1})
add_executable(${TEST1} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST1}.cpp)

set(TEST2 test_file64)
add_test(NAME ${TEST2} COMMAND ${TEST2})
add_executable(${TEST2} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST2}.cpp ${EXTERNAL_SOURCES})

set(TEST3 test_mysql64)
add_test(NAME ${TEST3} COMMAND ${TEST3})
add_executable(${TEST3} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST3}.cpp ${EXTERNAL_SOURCES})
