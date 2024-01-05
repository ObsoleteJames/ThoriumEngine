DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ ! -f "../engine/bin/linux/BuildTool" ]; then
	./build_buildtool.sh
fi

"../engine/bin/linux/BuildTool" "${DIR}/HeaderTool/Build.cfg"
cmake -B "HeaderTool/Intermediate/build" "HeaderTool/Intermediate/"
cmake --build "HeaderTool/Intermediate/build"
