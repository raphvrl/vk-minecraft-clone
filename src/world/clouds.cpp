#include "clouds.hpp"

namespace wld
{

void Clouds::init(gfx::Device &device)
{
    UNUSED(device);

    loadPattern("assets/textures/clouds.png");
    generateClouds();

    m_pipeline = gfx::Pipeline::Builder(device)
        .setShader("cloud.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("cloud.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setPushConstant(sizeof(glm::mat4))
        .setBlending(true)
        .setDepthTest(true)
        .build();
}

void Clouds::destroy()
{
    m_pipeline.destroy();
}

void Clouds::update(f32 dt)
{
    m_windOffset += WIND_SPEED * dt;

    f32 patternSizeX = m_width * CLOUD_SIZE;
    if (m_windOffset > patternSizeX) {
        m_windOffset -= patternSizeX;
    }
}

void Clouds::render(VkCommandBuffer cmd, const core::Camera &camera)
{
    m_pipeline.bind(cmd);

    glm::vec3 camPos = camera.getPos();

    f32 patternSizeX = m_width * CLOUD_SIZE;
    f32 patternSizeZ = m_height * CLOUD_SIZE;

    int cellX = std::floor(camPos.x / patternSizeX);
    int cellZ = std::floor(camPos.z / patternSizeZ);

    int renderedClouds = 0;
    f32 margin = CLOUD_SIZE;

    core::Frustum frustum = core::Frustum::fromViewProj(
        camera.getView(),
        camera.getProj()
    );

    for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
        for (int offsetX = -1; offsetX <= 1; offsetX++) {

            f32 baseOffsetX = (cellX + offsetX) * patternSizeX;
            f32 baseOffsetZ = (cellZ + offsetZ) * patternSizeZ;

            for (int gridZ = 0; gridZ < GRID_DIV; gridZ++) {
                for (int gridX = 0; gridX < GRID_DIV; gridX++) {

                    f32 subCellWidth = patternSizeX / GRID_DIV;
                    f32 subCellHeight = patternSizeZ / GRID_DIV;

                    glm::vec3 min(
                        baseOffsetX + gridX * subCellWidth - margin - m_windOffset,
                        CLOUD_ALTITUDE - margin,
                        baseOffsetZ + gridZ * subCellHeight - margin
                    );

                    glm::vec3 max(
                        baseOffsetX + (gridX + 1) * subCellWidth + margin - m_windOffset,
                        CLOUD_ALTITUDE + margin,
                        baseOffsetZ + (gridZ + 1) * subCellHeight + margin
                    );

                    if (!frustum.isBoxVisible(min, max)) {
                        continue;
                    }

                    CloudCell cell = {gridX, gridZ};
                    auto it = m_clouds.find(cell);

                    if (it != m_clouds.end()) {
                        for (const auto &cloud : it->second) {
                            glm::vec3 cloudPos = glm::vec3(
                                cloud.x + baseOffsetX - m_windOffset,
                                CLOUD_ALTITUDE,
                                cloud.y + baseOffsetZ
                            );

                            glm::vec3 toPlayer = camera.getPos() - cloudPos;
                            f32 dist = glm::length(toPlayer);

                            if (
                                dist < (CLOUD_SIZE * 8.0f * 2.0f) + 
                                CLOUD_ALTITUDE
                            ) {
                                glm::mat4 model = glm::mat4(1.0f);
                                model = glm::translate(model, cloudPos);

                                m_pipeline.push(cmd, model);

                                vkCmdDraw(cmd, 6, 1, 0, 0);
                                renderedClouds++;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Clouds::loadPattern(const std::string &path)
{
    stbi_set_flip_vertically_on_load(false);
    
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

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
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

    f32 cellWidth = static_cast<f32>(m_width) / GRID_DIV;
    f32 cellHeight = static_cast<f32>(m_height) / GRID_DIV;

    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            if (m_pattern[y * m_width + x]) {
                glm::vec2 cloudPos = {
                    static_cast<f32>(x) * CLOUD_SIZE,
                    static_cast<f32>(y) * CLOUD_SIZE
                };

                int gridX = static_cast<int>(x / cellWidth);
                int gridZ = static_cast<int>(y / cellHeight);

                gridX = std::min(gridX, GRID_DIV - 1);
                gridZ = std::min(gridZ, GRID_DIV - 1);

                CloudCell cell = {gridX, gridZ};
                m_clouds[cell].push_back(cloudPos);
            }
        }
    }
}

} // namespace wld