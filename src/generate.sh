./build_buildtool.sh

DIR="$( cd "$( dirname "$0" )" && pwd )"
"../engine/bin/linux/BuildTool" "${DIR}/ThoriumEngine/Build.cfg"
cmake -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TEST=OFF -DGLFW_BUILD_DOCS=OFF -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
