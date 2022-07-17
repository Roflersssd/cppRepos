cmake_minimum_required(VERSION 3.22)

project(pipleline)

set(CMAKE_CXX_STANDART 17)
file(GLOB CPP_SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${CPP_SOURCES})

