@echo off
"../engine/bin/win64/BuildTool.exe" %cd%"/ThoriumEngine/Build.cfg"
cmake -G "Visual Studio 16 2019" -A x64 -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
"../engine/bin/win64/BuildTool.exe" %cd%"/ThoriumEditor/Build.cfg"
cmake -G "Visual Studio 16 2019" -A x64 -B "ThoriumEditor/Intermediate/build" "ThoriumEditor/Intermediate"
pause