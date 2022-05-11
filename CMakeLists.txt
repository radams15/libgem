cmake_minimum_required(VERSION 3.22)
project(libgem C CXX)

set(CMAKE_C_STANDARD 99)

add_library(libgem STATIC src/main/Socket.c src/main/Socket.h src/main/Parser.cpp src/main/Parser.h)

find_package(OpenSSL REQUIRED)

add_executable(gemtest src/test/main.c)

target_link_libraries(libgem PRIVATE OpenSSL::SSL)

target_link_libraries(gemtest PRIVATE libgem)
target_include_directories(gemtest PRIVATE src/main)