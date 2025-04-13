# Changelog

All notable changes to the Vulkan Minecraft project will be documented in this file.

## [Unreleased]

### Added
- Sound effects for block placement and destruction for all block types
- Audio feedback system that varies sound based on block material

### Changed
- Enhanced sound system to support material-specific audio variations


## [0.2.0] - 2025-04-13

### Added
- Custom application icon for Windows and Linux
- Windows resource configuration for better integration with File Explorer
- Post-processing capability with framebuffers for storing rendering outputs
- Shading on block faces to enhance visual appearance
- Tree and flower generation in world terrain
- 3D sound system implementation
- Footstep sounds based on block type (partial implementation)
- Sound effects for UI button interactions

### Changed
- Upgraded to Vulkan 1.3
- Migrated build system to CMake
- Implemented dynamic rendering for improved performance
- Implemented bindless descriptor sets for more efficient resource binding
- Completely restructured graphics core architecture
- Simplified push constants implementation for pipeline configuration
- Increased gravity to better match Minecraft's physics

### Fixed
- Fixed water blocks appearing as half-blocks when another water block was directly above

## [0.1.0] - 2025-04-08

### Added
- Initial project setup with cross-platform build system (Windows/Linux)
- Vulkan rendering implementation
  - Swap chain management
  - Pipeline creation
  - Uniform buffers
  - Texture loading and management
  - Shader compilation and integration
- Graphics components
  - Block rendering
  - Skybox rendering
  - Cloud rendering
  - Basic lighting system
  - Fog effect
  - Outline highlighting for blocks
- World generation
  - Chunk-based terrain system
  - Procedural terrain generation with FastNoise
  - Block types with different textures
- Player systems
  - First-person camera controls
  - Basic movement (walking, jumping)
  - Water physics (buoyancy)
  - Collision detection
- GUI implementation
  - Text rendering
  - Basic UI elements
  - Pause menu
- Performance optimizations
  - Face culling for meshes
  - Frustum culling for chunks and clouds

[Unreleased]: https://github.com/raphvrl/vulkan-minecraft/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/raphvrl/vulkan-minecraft/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/raphvrl/vulkan-minecraft/releases/tag/v0.1.0