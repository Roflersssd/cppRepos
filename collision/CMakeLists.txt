cmake_minimum_required(VERSION 3.22)

project(colide)

file(GLOBAL CPP_SOURCE "src/*.cpp")

include_directories("${PROJECT_SOURCE_DIR/headers}")

add_executable(${PROJECT_NAME} CPP_SOURCE)
