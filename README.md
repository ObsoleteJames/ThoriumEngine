# Thorium Engine
A 3D game engine based on the Source Engine and Unreal Engine

![preview Image](preview.png)
### Planned Features
- Editable Meshes, including a modeling toolset for the editor.
- Compute Shaders.
- GPU Particle System.
- Custom Scripting Language.
- Render Graph, will replace the current rendering system.
- Networking System.

# Building
Requirements
- CMake 3.20.0 or greater
- MSVC 15 (Visual Studio 2017) or greater for windows
- GCC 11 or greater for linux

### Windows
- Run 'install_win64.bat' as administrator
- Run 'platforms/win64/build_buildtool.sh'
- Run 'platforms/win64/generate_vsproj.bat' this will generate a VS project in 'src/ThoriumEngine/Intermediate/build/'
- Open the generated project and compile the engine.

### Linux
- Run 'install_linux.sh'
- Run 'platforms/linux/build_release.sh' or 'platforms/linux/build_development.sh' to build the project
