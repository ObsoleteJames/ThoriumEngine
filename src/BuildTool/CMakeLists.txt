cmake_minimum_required(VERSION 3.4.0)
project(TH_BUILDTOOL)

include_directories(src ../Util/include)

add_subdirectory(../Util build)

add_executable(BuildTool src/main.cpp src/Build.cpp)
set_property(TARGET BuildTool PROPERTY CXX_STANDARD 17)
