@echo off
"../engine/bin/win64/BuildTool.exe" %cd%"/ThoriumEngine/Build.cfg" -development
cmake -A x64 -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
cmake --build "ThoriumEngine/Intermediate/build"
pause