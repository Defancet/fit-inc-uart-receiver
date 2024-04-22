# IZG GPU Implementation Project

## Author

- **Name:** Maksim Kalutski
- **Login:** xkalut00

## Overview

Task is to implement a simple graphics card (gpu). And also implement a function to render models. The project aims to
simulate the fundamental behaviors of a graphics processing unit (GPU) and includes the rendering of 3D models. The
primary task involves implementing various functions related to graphics processing such as command buffer management,
framebuffer operations, and shader programs.

## Project Structure

- `student/` - Main directory containing implementation files.
- `gpu.cpp` - Implements the `gpu_execute` function which simulates the GPU's functionality including rendering
  triangles and clearing the framebuffer.
- `drawModel.cpp` - Contains the `prepareModel`, `drawModel_vertexShader`, and `drawModel_fragmentShader` functions for
  processing model files and executing rendering commands.

## Implementation Details

- **Command Buffer Management** - Handles sequences of rendering or clearing commands.
- **Framebuffer Operations** - Supports operations like clearing and drawing to the framebuffer which consists of color
  and depth buffers.
- **Shader Functions** - Implements vertex and fragment shaders that process individual vertices and fragments.

## Building and Running

The project is set up using CMake and has been tested under Ubuntu with GCC and Windows with MSVC.

### Build Instructions

```bash
unzip izgProject.zip -d izgProject
cd izgProject/build
cmake ..
make
```

### Running Tests and Performance

- To run built-in acceptance tests: `./izgProject -c`
- To display help for command-line arguments: `./izgProject -h`
- For performance tests (recommended in release mode): `./izgProject -p`
