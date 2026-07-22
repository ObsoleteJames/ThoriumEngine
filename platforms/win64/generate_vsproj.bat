@echo off
"../../engine/bin/win64/BuildTool.exe" "../../src/ThoriumEngine/Build.cfg" -AS ./src
cmake -DFT_DISABLE_ZLIB=TRUE -DFT_DISABLE_BZIP2=TRUE -DFT_DISABLE_PNG=TRUE -DFT_DISABLE_HARFBUZZ=TRUE -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TEST=OFF -DGLFW_BUILD_DOCS=OFF -A x64 -B "../../src/ThoriumEngine/Intermediate/build" "../../src/ThoriumEngine/Intermediate"
