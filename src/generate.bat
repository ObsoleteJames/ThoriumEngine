@echo off
"../engine/bin/win64/BuildTool.exe" "ThoriumEngine/Build.cfg"
cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TEST=OFF -DGLFW_BUILD_DOCS=OFF -A x64 -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
"../engine/bin/win64/BuildTool.exe" "ThoriumEditor/Build.cfg"
cmake -DASSIMP_BUILD_TESTS=OFF -A x64 -B "ThoriumEditor/Intermediate/build" "ThoriumEditor/Intermediate"
