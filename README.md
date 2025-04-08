# Vulkan Minecraft Clone

A Minecraft clone built with Vulkan, compatible with both Windows and Linux.

## Installation

### Option 1: Prebuilt Release (Recommended)

1. Go to the [Releases page](https://github.com/raphvrl/vk-minecraft-clone/releases)
2. Find the latest version (e.g., v0.1.0-alpha)
3. Download the appropriate file for your system:
   - `vulkan-minecraft-windows-vX.X.X.zip` for Windows
   - `vulkan-minecraft-linux-vX.X.X.zip` for Linux
4. Extract the archive and run the game:
   - On Windows: run `vulkan-minecraft.exe`
   - On Linux: run `./vulkan-minecraft`

### Option 2: Building from Source

#### Prerequisites

**Windows:**
- Windows 10/11
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- MSYS2 with MinGW-w64

**Linux:**
- Recent distribution (Ubuntu 22.04+, Fedora 36+, etc.)
- Vulkan SDK: `sudo apt install libvulkan-dev vulkan-tools` (Ubuntu/Debian)
- Development dependencies:
  ```bash
  sudo apt install build-essential cmake glslang-tools libgl1-mesa-dev xorg-dev libwayland-dev libxkbcommon-dev
  ```

#### Build Steps

1. Clone the repository with its submodules:
   ```bash
   git clone --recursive https://github.com/raphvrl/vk-minecraft-clone.git
   cd vulkan-minecraft
   ```

2. Set up the environment:
   - **Windows**: Open MSYS2 MinGW 64-bit and set `VULKAN_SDK`:
     ```bash
     export VULKAN_SDK=/c/VulkanSDK/1.3.239.0  # Adjust to your installation
     ```
   - **Linux**: Set `VULKAN_SDK`:
     ```bash
     export VULKAN_SDK=/usr
     ```

3. Build the project:
   ```bash
   make
   ```

4. Run the game:
   - **Windows**: `./vulkan-minecraft.exe`
   - **Linux**: `./vulkan-minecraft`

## System Requirements

- Graphics card with Vulkan 1.3+ support
- Windows 10/11 or Linux with Vulkan support

## Controls

- WASD: Movement
- Space: Jump
- Mouse: Look around
- Left click: Destroy block
- Right click: Place block
- Escape: Pause menu / Quit

## Features

- Procedural terrain generation
- Dynamic lighting system
- Sky and cloud rendering
- Block system with different textures
- First-person controls