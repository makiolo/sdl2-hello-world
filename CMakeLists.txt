project(sdl2-hello-world CXX)
cmake_minimum_required(VERSION 3.0)
include(cmaki)
cmaki_find_package(sdl2)
cmaki_find_package(spdlog)
cmaki_find_package(ois)
cmaki_executable(test1 src/main.cpp PTHREADS)

