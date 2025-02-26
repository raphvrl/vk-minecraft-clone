# Vulkan Minecraft Clone

A Minecraft Clone built with Vulkan.

## Installation

### Option 1: Prebuilt Release (Recommended)

1. Go to the [Actions tab](https://github.com/raphvrl/vk-minecraft-clone/actions/workflows/build.yml)
2. Click on the most recent successful "Build" action
3. Scroll down and download the `build.zip` file under "Artifacts"
4. Extract the zip file and run `app.exe` to play the game

### Option 2: Building from Source

#### Prerequisites

- Windows 10/11
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- MinGW-w64 C++ compiler (through MSYS2)

#### Build Steps

1. Install the Vulkan SDK
2. Install MSYS2 and open MSYS2 MinGW 64-bit
3. Build the project:
```bash
git submodule update --init --recursive
make all
```
4. Run the game:
```bash
./app.exe
```
## Features

- [x] Vulkan initialization
- [x] Window management with GLFW
- [x] GLSL shader support
- [x] Infinite world generation with chunk system
- [x] Block face culling (including between chunks)
- [x] Perlin noise terrain generation for Minecraft Clone
- [x] Skybox implementation
- [x] Collision detection and physics
- [x] Entity Component System (ECS) architecture
- [ ] More coming soon...

## Development

This project is under active development. More features will be added as development progresses.

## Credits

The game textures are assets from Minecraft by Mojang. All Minecraft assets are property of Mojang AB. This project is not affiliated with Mojang AB.