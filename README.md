# Minecraft.cpp
## Requirements
- Google's Dawn is installed on your System
- CMake and a Build System is installed on your System
- The Rest of the Dependencies will be build for you
  
## Building
The following script configures and builds the cmake project
```shell
# This configures the Project for the Release version via cmake
$ cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release

# This builds the project
$ cmake --build build/release
```