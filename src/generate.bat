@echo off
"../engine/bin/BuildTool.exe" "ThoriumEngine/Build.cfg"
cmake -A x64 -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
"../engine/bin/BuildTool.exe" "ThoriumEditor/Build.cfg"
cmake -A x64 -B "ThoriumEditor/Intermediate/build" "ThoriumEditor/Intermediate"
