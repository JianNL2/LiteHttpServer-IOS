cmake_minimum_required(VERSION 3.9)
project(LiteHttpServer)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. DIR_SRCS)

add_executable(LiteHttpServer ${DIR_SRCS} main.cpp LiteHttpServer.cpp LiteHttpServer.h)