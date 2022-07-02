cmake_minimum_required(VERSION 3.22)

project(second_index)

file(GLOB CPP_SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${CPP_SOURCES})