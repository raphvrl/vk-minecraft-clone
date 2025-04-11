#pragma once

#include "core/types.hpp"
#include "graphics/buffer.hpp"
#include "graphics/device.hpp"
#include "core/camera/camera.hpp"

namespace gfx
{

constexpr u32 CAMERA_UBO = 0;
constexpr u32 TIME_UBO = 1;

struct CameraUBO
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 ortho;
    alignas(16) glm::vec3 position;
};

struct TimeUBO
{
    alignas(4) f32 time;
    alignas(4) f32 deltaTime;
};

class GPUData
{

public:
    void init(Device &deivce);
    void destroy();

    void updateCamera(const core::Camera &camera);
    void updateTime(f32 time, f32 deltaTime);

    void update();

private:
    Device *m_device = nullptr;

    Buffer m_cameraBuffer;
    Buffer m_timeBuffer;

    void createBuffers();
};

}