@echo off
"../../engine/bin/win64/BuildTool.exe" "../../src/ThoriumEngine/Build.cfg"
cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TEST=OFF -DGLFW_BUILD_DOCS=OFF -A x64 -B "../../src/ThoriumEngine/Intermediate/build" "../../src/ThoriumEngine/Intermediate"
