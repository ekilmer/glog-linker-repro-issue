cmake_minimum_required(VERSION 3.22)

project(glog-linker-repro)

find_package(glog CONFIG REQUIRED)

add_executable(issue main.cpp)
target_link_libraries(issue PRIVATE glog::glog)
