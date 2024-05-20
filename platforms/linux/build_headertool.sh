if [ ! -f "../../engine/bin/linux/BuildTool" ]; then
	./build_buildtool.sh
fi

"../../engine/bin/linux/BuildTool" "../../src/HeaderTool/Build.cfg"
cmake -B "../../src/HeaderTool/Intermediate/build" "../../src/HeaderTool/Intermediate/"
cmake --build "../../src/HeaderTool/Intermediate/build"
