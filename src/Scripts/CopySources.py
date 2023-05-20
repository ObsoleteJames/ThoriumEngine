
# Copies project binaries into build folder for public use.
# Usage - CopySources.py [PROJECT_NAME] [BUILD_TYPE] [PLATFORM] [SOURCE_FOLDER] [BINARIES_PATH] [BUILD_PATH]

import shutil
import sys

def getHeaderFiles(path):
	return 0

def main():
	if (sys.argv.count < 7):
		print("Invalid number of agruments")
		exit(1)
	
	projectName = sys.argv[1]
	buildType = sys.argv[2]
	platform = sys.argv[3]
	sourceFolder = sys.argv[4]
	binaryPath = sys.argv[5]
	buildPath = sys.argv[6]

	outPath = buildPath + "/" + platform + "/" + projectName + "-" + buildType

	# Copy module file to build folder
	moduleBin = binaryPath + "/module.bin"
	moduleBinOut = outPath + "/module.bin"
	shutil.copyfile(moduleBin, moduleBinOut)

	# Copy binaries
	libFile = binaryPath + "/" + platform + "/" + buildType + "/" + projectName + ".lib"
	shutil.copyfile(libFile, outPath + "/lib/" + projectName + ".lib")
	
	# we dont know if the given application is an exe or a dll so we try both.
	libFile = binaryPath + "/" + platform + "/" + buildType + "/" + projectName + ".exe"
	shutil.copyfile(libFile, outPath + "/lib/" + projectName + ".exe")
	
	libFile = binaryPath + "/" + platform + "/" + buildType + "/" + projectName + ".dll"
	shutil.copyfile(libFile, outPath + "/lib/" + projectName + ".dll")
	
if __name__ == "__main__":
	main()
