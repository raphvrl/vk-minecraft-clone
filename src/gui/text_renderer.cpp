#include "text_renderer.hpp"

namespace gui
{

void TextRenderer::init(gfx::Device &device)
{
    m_device = &device;

    m_texture = m_device->loadImage(
        "assets/textures/font.png",
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    );

    m_textureID = m_device->addTexture(m_texture);

    m_pipeline = gfx::Pipeline::Builder(device)
        .setShader("text.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("text.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .addPushConstantRange({
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = sizeof(PushConstant)
        })
        .setBlending(true)
        .setDepthTest(false)
        .setDepthWrite(false)
        .build();

    calculateCharWidths();
}

void TextRenderer::destroy()
{
    m_texture.destroy();
    m_pipeline.destroy();
}

void TextRenderer::draw(
    VkCommandBuffer cmd,
    const std::string &text,
    const glm::vec2 &pos,
    u32 size,
    TextAlign align
)
{
    m_pipeline.bind(cmd);

    const f32 pixelOffset = static_cast<float>(size) / 8.0f;

    int textWidth = 0;
    for (unsigned char c : text) {
        textWidth += m_charWidths[c] * pixelOffset;
    }

    int x = pos.x;
    int y = pos.y;

    switch (align)
    {
    case TextAlign::LEFT:
        break;

    case TextAlign::RIGHT:
        x -= textWidth;
        break;

    case TextAlign::CENTER:
        x -= textWidth / 2;
        break;
    }
        

    x = std::floor(x);

    for (unsigned char c : text) {
        int col = c % 16;
        int row = c / 16;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(
            x + pixelOffset,
            y + pixelOffset,
            0.0f
        ));
        model = glm::scale(model, glm::vec3(size, size, 1.0f));

        PushConstant pc;
        pc.model = model;
        pc.uv = glm::vec2(
            static_cast<f32>(col) / 16.0f,
            static_cast<f32>(row) / 16.0f
        );
        pc.color = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
        pc.textureID = m_textureID;

        m_pipeline.push(
            cmd,
            static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
            sizeof(pc),
            &pc
        );

        vkCmdDraw(cmd, 6, 1, 0, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(size, size, 1.0f));

        pc.model = model;
        pc.color = glm::vec4(1.0f);

        m_pipeline.push(
            cmd,
            static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
            sizeof(pc),
            &pc
        );

        vkCmdDraw(cmd, 6, 1, 0, 0);

        x += m_charWidths[c] * pixelOffset;
    }
}

void TextRenderer::calculateCharWidths()
{
    stbi_set_flip_vertically_on_load(false);

    int width, height, channels;
    stbi_uc *pixels = stbi_load(
        "assets/textures/font.png",
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
    );

    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    const int charWidth = width / 16;
    const int charHeight = height / 16;

    m_charWidths.fill(6);
    
    for (int i = 0; i < 256; i++) {
        int col = i % 16;
        int row = i / 16;

        int startX = col * charWidth;
        int startY = row * charHeight;

        int actualWidth = getCharWidth(
            pixels,
            width,
            height,
            channels,
            startX,
            startY,
            charWidth,
            charHeight
        );

        m_charWidths[i] = actualWidth + 1;
    }

    m_charWidths[' '] = 4;
    stbi_image_free(pixels);
}

int TextRenderer::getCharWidth(
    stbi_uc *pixels,
    int imgWidth,
    int imgHeight,
    int channels,
    int startX,
    int startY,
    int charWidth,
    int charHeight
)
{
    UNUSED(imgHeight);

    for (int x = charWidth - 1; x >= 0; x--) {
        for (int y = 0; y < charHeight; y++) {
            int pixelX = startX + x;
            int pixelY = startY + y;

            int index = (pixelY * imgWidth + pixelX) * channels;
            if (pixels[index + 3] > 127) {
                return x + 1;
            }
        }
    }

    return 0;
}

} // namespace gfx