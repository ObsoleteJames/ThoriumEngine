@echo off
./generate_vsproj.bat
cmake -CMAKE_BUILD_TYPE=RelWithDebugInfo --build "../../src/ThoriumEngine/Intermediate/build"
pause