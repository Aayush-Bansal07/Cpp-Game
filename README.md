# 3D OpenGL Game

A simple 3D game built with OpenGL, GLEW, and GLFW that renders a rotating cube you can move around.

## Features

- Modern OpenGL 3.3 core pipeline with shaders
- 3D cube rendering with depth testing
- Textured cubes with basic lighting
- Multiple objects in the scene
- Smooth keyboard-based movement controls
- Mouse-look camera system
- Collision detection with cubes
- Window management with GLFW

## Prerequisites

You need to have the following libraries installed:

- **GLFW3** - Window and input handling
- **GLEW** - OpenGL Extension Wrangler
- **OpenGL** - Graphics rendering

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libglfw3-dev libglew-dev libgl1-mesa-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install glfw-devel glew-devel mesa-libGL-devel
```

**macOS (with Homebrew):**
```bash
brew install glfw glew
```

**Windows:**
- Download GLFW from [glfw.org](https://www.glfw.org/)
- Download GLEW from [glew.sourceforge.net](http://glew.sourceforge.net/)
- Follow their installation instructions

## Building

### Linux/macOS

```bash
g++ -o game main.cpp -lGL -lGLEW -lglfw -lm
```

### Windows (MinGW)

```bash
g++ -o game.exe main.cpp -lglfw3 -lglew32 -lopengl32 -lgdi32
```

## Running

```bash
./game
```

On Windows:
```bash
game.exe
```

## Controls

- **W** - Move forward
- **S** - Move backward
- **A** - Move left
- **D** - Move right
- **Space** - Move up
- **Left Shift** - Move down
- **Mouse** - Look around
- **Q / E** - Rotate cubes around Y-axis
- **R / F** - Rotate cubes around X-axis
- **Z / C** - Rotate cubes around Z-axis
- **Esc** - Exit

## Code Structure

- Window creation: 800x600 pixels
- Initial camera position: (0, 0, -5)
- Cube size: 2x2x2 units
- Movement speed: 0.05 units per frame
- Color: Light blue (RGB: 0.2, 0.7, 1.0)

## Notes

- This project uses the modern OpenGL pipeline (shaders, VAO/VBO)
- The cubes are centered at their positions with size 1x1x1
- Depth testing is enabled for proper 3D rendering

## Future Improvements

- Add skybox and environment lighting
- Add specular highlights and materials
- Add textures from image files
- Implement object picking and interactions
- Add physics-based collisions

## License

This is a simple demonstration project for educational purposes.
