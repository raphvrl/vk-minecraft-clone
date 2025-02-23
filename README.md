# Vulkan Minecraft Clone

A Minecraft-like game built with Vulkan.

## Requirements

- Windows 10/11
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- MinGW-w64 C++ compiler (through MSYS2)

## Setup & Build

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
- [ ] More coming soon...

## Development

This project is under active development. More features will be added as development progresses.