@echo off
"../../engine/bin/win64/BuildTool.exe" "../../src/ThoriumEditor/Build.cfg" -development
cmake -A x64 -B "../../src/ThoriumEditor/Intermediate/build" "../../src/ThoriumEditor/Intermediate"
cmake --build "../../src/ThoriumEditor/Intermediate/build"
pause