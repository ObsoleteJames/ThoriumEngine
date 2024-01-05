./build_buildtool.sh

DIR="$( cd "$( dirname "$0" )" && pwd )"
"../engine/bin/linux/BuildTool" "${DIR}/ThoriumEngine/Build.cfg"
cmake -B "ThoriumEngine/Intermediate/build" "ThoriumEngine/Intermediate"
