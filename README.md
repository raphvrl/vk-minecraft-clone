# Vulkan Minecraft Clone

A Minecraft-like game built with Vulkan.

## Installation

### Option 1: Prebuilt Release (Recommended)

A prebuilt version of the game is available through GitHub Actions:

1. Go to the "Actions" tab of this repository
2. Select the latest successful "Build" workflow
3. Download the "build" artifact
4. Extract the archive which contains:
   - `app.exe` - The game executable
   - `assets/` - Required game resources

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
- [x] Perlin noise terrain generation for Minecraft-like world
- [ ] More coming soon...

## Development

This project is under active development. More features will be added as development progresses.