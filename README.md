# VulkanEngine

A render engine created using Vulkan. A good name has yet to be decided. Ported over to windows with visual studio due to some development troubles with ubuntu


## Building And Running
Before following the build instructions make sure that you have:
* CMake (ver. 3.16.3 or greater)
* Vulkan
* GLFW

You can follow the instructions in https://vulkan-tutorial.com/Development_environment to get Vulkan and GLFW setup


Go the directory where you cloned this repository and run these command in order:
```
mkdir ./build
cd build/
cmake ..
make
./main
```

## Current TODO List
1. Camera Movement
2. Lighting
3. Texturing
