"../../engine/bin/win64/BuildTool.exe" "../../src/HeaderTool/Build.cfg" -release
cmake -A x64 -B "../../src/HeaderTool/Intermediate/build" "../../src/HeaderTool/Intermediate"
cmake --build "../../src/HeaderTool/Intermediate/build"