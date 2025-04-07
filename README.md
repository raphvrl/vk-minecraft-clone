# Vulkan Minecraft Clone

A Minecraft Clone built with Vulkan.

## Installation

### Option 1: Prebuilt Release (Recommended)

1. Go to the [Releases page](https://github.com/raphvrl/vk-minecraft-clone/releases)
2. Find the latest release version (e.g., v0.1.0)
3. Download the zip file for your platform (e.g., `vulkan-minecraft-windows-x64.zip`)
4. Extract the zip file and run `vulkan-minecraft.exe` to play the game

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