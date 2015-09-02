project(ApplicationsServer)
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE C_F "src/*.c")
file(GLOB_RECURSE CPP_F "src/*.cpp")

add_executable(${PROJECT_NAME} ${HEADERS} ${C_F} ${CPP_F})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -ggdb")
