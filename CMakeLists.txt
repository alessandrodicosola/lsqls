cmake_minimum_required(VERSION 3.0)

project(lsqls)

set(CMAKE_CXX_STANDARD 17)

if(UNIX)
set(OS linux)
elseif(WIN)
set(OS win)
endif(UNIX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/${OS}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/${OS}/bin/debug)

set(MAIN_SOURCES  ${CMAKE_SOURCE_DIR}/src/main.cpp)
set(SOURCES 
            
            ${CMAKE_SOURCE_DIR}/src/file64.cpp
)

add_definitions(-D_FILE_OFFSET_BITS=64)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(lsqls ${MAIN_SOURCES} ${SOURCES})

if(UNIX)
#LFS
target_link_libraries(lsqls PRIVATE "stdc++fs")
endif(UNIX)

#TEST
enable_testing()


add_subdirectory(${CMAKE_SOURCE_DIR}/test)


