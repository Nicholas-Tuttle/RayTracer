# Ray Tracer
This project is a CPU and GPU ray tracer using the Vulkan API. It is a heavy work-in-progress and lots of features need to be implemented (materials, anything other than spheres being rendered, lights, etc.). The CPU and GPU rendering outputs are not always in sync as things are being added.
## Rendering Example:
![image](./image.png)
## Roadmap:
![image](./doc/Roadmap.png)

```mermaid
  journey
    title Roadmap
    section The Basics
      Basic Materials: 5: Materials
      Lights: 5: Lighting And Shadows
      Scene Importing And Rendering: 5: External Interfaces
      Blender Integration: 3: External Interfaces
    section More Advanced
      HDRI: 4: Lighting And Shadows
      Textures: 5: Materials
      Advanced Surface Materials: 3: Materials
    section Future
      Performance Improvements And Refactors: 3: Performance
      Volumes: 4: Materials
      Animations: 2: External Interfaces
```

# Dependencies:
## LibPNG:
http://www.libpng.org/pub/png/libpng.html
## ZLib:
http://www.zlib.net/
## Vulkan SDK:
https://www.lunarg.com/vulkan-sdk/
# Installaton
## Required Environment Variables for Visual Studio solution:
ZLIB - zlib library source directory

LIBPNG - lpng1637 library source directory 

VULKAN_SDK - Vulkan SDK

## Compatibility

This has only been built and tested on Windows
