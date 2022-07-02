cmake_minimum_required(VERSION 3.22)

project(priority_collection_2)

file(GLOB CPP_SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${CPP_SOURCES})