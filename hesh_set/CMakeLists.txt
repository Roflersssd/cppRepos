cmake_minimum_required(VERSION 3.22)

project(hesh_set)

file(GLOB CPP_SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${CPP_SOURCES})