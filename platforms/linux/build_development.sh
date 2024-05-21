./build_buildtool.sh
./build_headertools.sh

ROOT="../.."
SRC="../../src"
ENGINE="../../engine"

"${ENGINE}/bin/linux/BuildTool" "${SRC}/ThoriumEngine/Build.cfg" -development -AS "./src"
cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TEST=OFF -DGLFW_BUILD_DOCS=OFF -B "${SRC}/ThoriumEngine/Intermediate/Build" "${SRC}/ThoriumEngine/Intermediate"
cmake --build "ThoriumEngine/Intermediate/build"
