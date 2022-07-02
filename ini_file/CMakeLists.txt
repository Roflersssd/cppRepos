cmake_minimum_required(VERSION 3.22)

project(ini_file)

file(GLOB CPP_SOURCES "src/*.cpp")

include_directories("${PROJECT_SOURCE_DIR}"/include)

add_executable(${PROJECT_NAME} ${CPP_SOURCES})