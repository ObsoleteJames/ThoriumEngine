@echo off
call generate_vsproj.bat
"../../engine/bin/win64/BuildTool.exe" "../../src/ThoriumEditor/Build.cfg"
cmake -DASSIMP_BUILD_TESTS=OFF -A x64 -B "../../src/ThoriumEditor/Intermediate/build" "../../src/ThoriumEditor/Intermediate"
