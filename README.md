# Ray Tracer
This project is a CPU and GPU ray tracer using the Vulkan API. It is a heavy work-in-progress and lots of features need to be implemented (materials, anything other than spheres being rendered, lights, etc.). The CPU and GPU rendering outputs are not always in sync as things are being added.
## Rendering Example:
![image](./image.png)
## Roadmap:
```mermaid
  journey
    title Roadmap
    section The Basics
      Basic Materials: 5: Materials
      Lights: 5: Lighting
      Scene Importing And Rendering: 5: Interfaces
      Blender Integration: 3: Interfaces
    section More Advanced
      HDRI: 4: Lighting
      Textures: 5: Materials
      Advanced Surface Materials: 3: Materials
    section Future
      Performance Improvements: 3: Performance
      Volumes: 4: Materials
      Animations: 2: Interfaces
```
## Vulkan SDK:
https://www.lunarg.com/vulkan-sdk/
# Installaton
## Required Environment Variables for Visual Studio solution:
VULKAN_SDK - Vulkan SDK installation location

All other dependencies are included as submodules, use "git clone --recursive" to get them

## Compatibility

This has only been built and tested on Windows
