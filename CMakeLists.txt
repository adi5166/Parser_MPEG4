cmake_minimum_required(VERSION 3.16)
project(Parser_MPEG4)

set(CMAKE_CXX_STANDARD 14)

add_executable(Parser_MPEG4 TS_parser.cpp tsTransportStream.cpp tsTransportStream.h tsCommon.h)