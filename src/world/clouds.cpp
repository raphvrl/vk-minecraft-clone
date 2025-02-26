#include "clouds.hpp"

namespace wld
{

void Clouds::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    loadPattern("assets/textures/clouds.png");
    generateClouds();

    m_pipeline = gfx::Pipeline::Builder(ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "cloud.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "cloud.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count = 1,
            .stage = VK_SHADER_STAGE_VERTEX_BIT
        })
        .addPushConstant(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4)
        )
        .build();

    m_ubo.init(ctx, sizeof(UniformBufferObject));

    std::vector<gfx::DescriptorData> descriptors = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .ubo = &m_ubo
        }
    };

    m_descriptorSet = m_pipeline.createDescriptorSet(descriptors);
}

void Clouds::destroy()
{
    m_ubo.destroy();
    m_pipeline.destroy();
}

void Clouds::update(f32 dt)
{
    m_windOffset += WIND_SPEED * dt;
}

void Clouds::render(const core::Camera &camera)
{
    VkCommandBuffer cmd = m_ctx->getCommandBuffer();

    m_pipeline.bind();
    m_pipeline.bindDescriptorSet(m_descriptorSet);

    glm::vec3 camPos = camera.getPos();

    UniformBufferObject uniformBuffer = {
        .view = camera.getView(),
        .proj = camera.getProj(),
        .camPos = camPos
    };

    m_ubo.update(&uniformBuffer, sizeof(UniformBufferObject));

    f32 patternSizeX = m_width * CLOUD_SIZE;
    f32 patternSizeZ = m_height * CLOUD_SIZE;

    i32 cellX = std::floor(camPos.x / patternSizeX);
    i32 cellZ = std::floor(camPos.z / patternSizeZ);

    for (i32 offsetZ = -1; offsetZ <= 1; offsetZ++) {
        for (i32 offsetX = -1; offsetX <= 1; offsetX++) {

            f32 baseOffsetX = (cellX + offsetX) * patternSizeX;
            f32 baseOffsetZ = (cellZ + offsetZ) * patternSizeZ;

            for (const glm::vec2 &cloud : m_clouds) {
                glm::vec3 cloudPos = glm::vec3(
                    baseOffsetX + cloud.x - m_windOffset,
                    CLOUD_ALTITUDE,
                    baseOffsetZ + cloud.y
                );

                glm::vec3 toPlayer = camera.getPos() - cloudPos;
                f32 dist = glm::length(toPlayer);

                if (dist < (CLOUD_SIZE * 8.0f * 2.0f) + CLOUD_ALTITUDE) {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, cloudPos);

                    m_pipeline.push(
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(model),
                        &model
                    );

                    vkCmdDraw(cmd, 6, 1, 0, 0);
                }
            }
        }
    }
}

void Clouds::loadPattern(const std::string &path)
{
    int width, height, channels;
    stbi_uc *pixels = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    m_width = width;
    m_height = height;
    m_pattern.resize(width * height);

    for (i32 y = 0; y < height; y++) {
        for (i32 x = 0; x < width; x++) {
            int index = (y * width + x) * 4;
            bool isCloud = pixels[index + 3] > 127;

            m_pattern[y * width + x] = isCloud;
        }
    }

    stbi_image_free(pixels);
}

void Clouds::generateClouds()
{
    m_clouds.clear();

    for (i32 y = 0; y < m_height; y++) {
        for (i32 x = 0; x < m_width; x++) {
            if (m_pattern[y * m_width + x]) {
                m_clouds.push_back({
                    static_cast<f32>(x) * CLOUD_SIZE,
                    static_cast<f32>(y) * CLOUD_SIZE
                });
            }
        }
    }
}

} // namespace wld