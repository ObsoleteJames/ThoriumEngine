@echo off
"../engine/bin/win64/BuildTool.exe" "ThoriumEngine/Build.cfg"
cmake -GLFW_BUILD_EXAMPLES=OFF -GLFW_BUILD_TEST=OFF -GLFW_BUILD_DOCS=OFF -A x64 -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
"../engine/bin/win64/BuildTool.exe" "ThoriumEditor/Build.cfg"
cmake -ASSIMP_BUILD_TESTS=OFF -A x64 -B "ThoriumEditor/Intermediate/build" "ThoriumEditor/Intermediate"
