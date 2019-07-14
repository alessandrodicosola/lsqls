cmake_minimum_required(VERSION 3.0)

project(lsqls)

set(CMAKE_CXX_STANDARD 17)
#LFS
add_definitions(-D_FILE_OFFSET_BITS=64)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(MAIN_SOURCES  ${CMAKE_SOURCE_DIR}/src/main.cpp)
set(SOURCES 
            #${CMAKE_SOURCE_DIR}/src/sql_file.cpp
            ${CMAKE_SOURCE_DIR}/src/file64.cpp
            #${CMAKE_SOURCE_DIR}/src/statement.cpp
)



include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(lsqls ${MAIN_SOURCES} ${SOURCES})
target_link_libraries(lsqls PRIVATE "stdc++fs")

#TEST
enable_testing()


add_subdirectory(${CMAKE_SOURCE_DIR}/test)


