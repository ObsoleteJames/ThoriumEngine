@echo off
./generate_vsproj.bat
cmake -CMAKE_BUILD_TYPE=Release --build "../../src/ThoriumEngine/Intermediate/build"
pause