cmake_minimum_required(VERSION 3.20.0)
project(TH_BUILDTOOL)

include_directories(src ../Util/include)

add_subdirectory(../Util build)

add_executable(BuildTool src/main.cpp src/Build.cpp)
set_property(TARGET BuildTool PROPERTY CXX_STANDARD 17)

target_link_libraries(BuildTool Util)

if (WIN32)
    add_custom_command(TARGET BuildTool POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
        "$<TARGET_FILE:BuildTool>" "../../../../engine/bin/win64/BuildTool.exe")
else ()
    add_custom_command(TARGET BuildTool POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
        "$<TARGET_FILE:BuildTool>" "../../../../engine/bin/linux/BuildTool")
endif (WIN32)
