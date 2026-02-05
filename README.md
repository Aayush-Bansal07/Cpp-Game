# 3D Overworld (OpenGL)

A small first-person overworld scene built with modern OpenGL (3.3 core), GLEW, and GLFW. You can explore a tiled ground, walk around tinted cubes, and see simple lighting with fog for depth.

## Features

- 1600x900 window with dynamic viewport sizing
- Textured, lit cubes and a large ground plane
- Ambient + diffuse + specular lighting with per-object color tints
- Exponential fog for atmospheric depth
- Mouse-look camera with collision against world props
- Basic checkerboard procedural texture (replaceable with real assets)

## Prerequisites

- OpenGL 3.3 capable GPU/driver
- GLFW3 (window + input)
- GLEW (extension loading)

### Install (Linux/macOS)

```bash
sudo apt-get update && sudo apt-get install -y libglfw3-dev libglew-dev libgl1-mesa-dev   # Debian/Ubuntu
sudo dnf install -y glfw-devel glew-devel mesa-libGL-devel                                # Fedora/RHEL
brew install glfw glew                                                                    # macOS (Homebrew)
```

### Install (Windows)

- Grab prebuilt binaries: [GLFW](https://www.glfw.org/) and [GLEW](http://glew.sourceforge.net/)
- Link against `glfw3`, `glew32`, and `opengl32` in your toolchain

## Build

Assuming source file is `game.cpp` in the project root.

```bash
g++ -std=c++17 -O2 -o game game.cpp -lGL -lGLEW -lglfw -lm               # Linux
g++ -std=c++17 -O2 -o game game.cpp -framework OpenGL -lglew -lglfw      # macOS
g++ -std=c++17 -O2 -o game.exe game.cpp -lglfw3 -lglew32 -lopengl32 -lgdi32  # Windows (MinGW)
```

## Run

```bash
./game      # Linux/macOS
game.exe    # Windows
```

## Controls

- W / A / S / D: Move
- Space / Left Shift: Move up / down
- Mouse: Look around
- Q / E: Rotate cubes (Y axis)
- R / F: Rotate cubes (X axis)
- Z / C: Rotate cubes (Z axis)
- Esc: Quit

## Technical Notes

- Ground: tiled plane scaled to 40x40 with its own color tint
- Props: palette-tinted cubes with basic sphere-AABB camera collision
- Lighting: single directional light; Phong specular; per-object tint
- Fog: exponential; tweak density via `uFogDensity` uniform (default 0.03)
- Texture: procedural 64x64 checker; swap in real textures by replacing the upload code

## Next Ideas

- Skybox and sun/sky gradients
- Real texture assets (grass, rocks, crates)
- Simple pickups or triggers in the overworld
- Post-processing (bloom/tonemap) if you extend the pipeline

## License

This is a simple demonstration project for educational purposes.
