cmake_minimum_required(VERSION 3.4.0)
project(TH_UTIL)

include_directories(src)

add_library(Util STATIC 
    src/Assert.cpp 
    src/FStream.cpp
    src/Guid.cpp
    src/KeyValue.cpp
    src/MemStream.cpp
    src/String.cpp)

set_property(TARGET Util PROPERTY CXX_STANDARD 17)
