@echo off
"../engine/bin/win64/BuildTool.exe" "ThoriumEngine/Build.cfg" -development
cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TEST=OFF -DGLFW_BUILD_DOCS=OFF -A x64 -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
