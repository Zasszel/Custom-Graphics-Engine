# Interactive OpenGL Graphics Engine

A lightweight, custom-built 3D engine developed in C++ and OpenGL 3.3. This project focuses on real-time object interaction, dynamic effects, and providing an accessible interface for scene manipulation.

## Features

* **3D Raycasting:** Implements screen-to-world space transformation (NDC -> Clip -> Eye -> World) to allow precision mouse interaction and object selection.
* **Dynamic Particle System:** A custom "burst" physics system for explosions with real-time lifecycle management and alpha-fade scaling.
* **Custom Shader Pipeline:** A modular `Shader` class handling vertex and fragment programs, supporting Phong-like lighting, texture mapping, and uniform management.
* **Live UI Control:** Fully integrated **ImGui** dashboard to tweak engine parameters (object count, spawn displacement) and light positions in real-time.

## Tech Stack

| Component | Library |
| :--- | :--- |
| **Language** | C++17 |
| **Graphics API** | OpenGL 3.3 (Core Profile) |
| **Windowing & Input** | GLFW |
| **OpenGL Loader** | GLAD |
| **Mathematics** | GLM (OpenGL Mathematics) |
| **GUI** | Dear ImGui |
| **Image Loading** | stb_image |

## Controls & Interaction

* **Left Click:** Select and "Destroy" 3D objects in the scene.
* **Engine UI:**
* * `Object Count`: Set how many objects spawn.
    * `Displacement`: Adjust the randomness of object positions.
    * `Regenerate`: Clear and respawn the scene.
    * `Light XYZ`: Move the global light source.
