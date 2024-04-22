@echo off
"../engine/bin/win64/BuildTool.exe" %cd%"/ThoriumEditor/Build.cfg" -development
cmake -A x64 -B "ThoriumEditor/Intermediate/build" "ThoriumEditor/Intermediate"
cmake --build "ThoriumEditor/Intermediate/build"
pause