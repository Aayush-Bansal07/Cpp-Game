# 3D OpenGL Game

A simple 3D game built with OpenGL, GLEW, and GLFW that renders a rotating cube you can move around.

## Features

- 3D cube rendering with depth testing
- Smooth keyboard-based movement controls
- Uses legacy OpenGL fixed-function pipeline
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

- **W** - Move forward (increase Z position)
- **S** - Move backward (decrease Z position)
- **A** - Move left (decrease X position)
- **D** - Move right (increase X position)

## Code Structure

- Window creation: 800x600 pixels
- Initial camera position: (0, 0, -5)
- Cube size: 2x2x2 units
- Movement speed: 0.05 units per frame
- Color: Light blue (RGB: 0.2, 0.7, 1.0)

## Notes

- This project uses the legacy OpenGL fixed-function pipeline (immediate mode)
- For modern OpenGL applications, consider using shaders and vertex buffer objects
- The cube is centered at the origin (0, 0, 0)
- Depth testing is enabled for proper 3D rendering

## Future Improvements

- Add rotation controls
- Implement modern OpenGL with shaders
- Add lighting and textures
- Camera system with mouse look
- Multiple objects in the scene
- Collision detection

## License

This is a simple demonstration project for educational purposes.
