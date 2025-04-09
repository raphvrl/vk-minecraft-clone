#include "gpu_data.hpp"

namespace gfx
{

void GPUData::init(Device &device)
{
    m_device = &device;

    createBuffers();
}

void GPUData::destroy()
{
    m_device->waitIdle();
    m_cameraBuffer.destroy();
}

void GPUData::updateCamera(const core::Camera &camera)
{
    auto data = static_cast<CameraUBO *>(m_cameraBuffer.map());

    data->view = camera.getView();
    data->proj = camera.getProj();
    data->ortho = camera.getOrtho();
    data->position = camera.getPos();

    m_cameraBuffer.unmap();
}

void GPUData::update()
{
    m_device->update();
}

void GPUData::createBuffers()
{
    m_cameraBuffer = m_device->createBuffer(
        sizeof(CameraUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    CameraUBO defaultData;
    defaultData.view = glm::mat4(1.0f);
    defaultData.proj = glm::mat4(1.0f);
    defaultData.ortho = glm::mat4(1.0f);
    defaultData.position = glm::vec3(0.0f);

    auto data = static_cast<CameraUBO *>(m_cameraBuffer.map());
    *data = defaultData;
    m_cameraBuffer.unmap();

    u32 cameraID = m_device->addUBO(m_cameraBuffer);
    if (cameraID != CAMERA_UBO) {
        throw std::runtime_error("Failed to add camera UBO to bindless manager!");
    }

    m_device->update();
}

} // namespace gfx