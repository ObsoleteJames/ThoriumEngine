./build_buildtool.sh

DIR="$( cd "$( dirname "$0" )" && pwd )"
"../engine/bin/linux/BuildTool" "${DIR}/ThoriumEngine/Build.cfg"
cmake -GLFW_BUILD_EXAMPLES=OFF -GLFW_BUILD_TEST=OFF -GLFW_BUILD_DOCS=OFF -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
