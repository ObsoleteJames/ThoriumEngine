DIR="$( cd "$( dirname "$0" )" && pwd )"
cmake -B "BuildTool/Intermediate/Build" "BuildTool"
cmake --build "BuildTool/Intermediate/Build"

"../engine/bin/linux/BuildTool" "${DIR}/ThoriumEngine/Build.cfg"
cmake -B "ThoriumEngine/Intermediate/build/" "ThoriumEngine/Intermediate"
