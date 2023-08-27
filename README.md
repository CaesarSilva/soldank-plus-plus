# Soldat++
C++23 implementation of [Soldat](https://github.com/soldat/soldat)

### Requirements
The project is written in C++23 and depends on:
- OpenGL 2.1+
- glad
- glfw-3.3.8
- glm
- stb
- FreeType

### Building with vcpkg
Make sure you have CMake installed.
To install all the required package using vcpkg:
```
vcpkg install glfw3 glm stb freetype simpleini --triplet x64-windows
mkdir build
cd build
cmake ..
```

On linux:
```
sudo vcpkg install glfw3 glm stb freetype simpleini --triplet x64-linux
mkdir build
cd build
cmake ..
```

You should now be able to build executables using `make` on linux or to open generated solution in visual studio on windows
