cmake_minimum_required(VERSION 3.22)

project(expressions_tree)

set(CMAKE_CXX_STANDART 17)
file(GLOB CPP_SOURCES "src/*.cpp")

include_directories("${PROJECT_SOURCE_DIR}"/headers)

add_executable(${PROJECT_NAME} ${CPP_SOURCES})

