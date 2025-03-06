#include "text_renderer.hpp"

namespace gui
{

void TextRenderer::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = gfx::Pipeline::Builder(ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "text.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "text.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count = 1,
            .stage = VK_SHADER_STAGE_VERTEX_BIT
        })
        .addDescriptorBinding({
            .binding = 1,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT
        })
        .addPushConstant(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstant)
        )
        .setBlending(true)
        .setDepthTest(false)
        .setDepthWrite(false)
        .build();

    m_font.init(*m_ctx, "font.png", false);
    m_ubo.init(*m_ctx, sizeof(UniformBufferObject));

    std::vector<gfx::DescriptorData> descriptors = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .ubo = &m_ubo
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .binding = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .texture = &m_font
        },
    };

    m_descriptorSet = m_pipeline.createDescriptorSet(descriptors);

    calculateCharWidths();
}

void TextRenderer::destroy()
{
    m_ubo.destroy();
    m_font.destroy();
    m_pipeline.destroy();
}

void TextRenderer::draw(
    const std::string &text,
    const glm::vec2 &pos,
    u32 size,
    TextAlign align
)
{
    VkExtent2D extent = m_ctx->getSwapChainExtent();
    VkCommandBuffer cmd = m_ctx->getCommandBuffer();

    m_pipeline.bind();
    m_pipeline.bindDescriptorSet(m_descriptorSet);

    glm::mat4 ortho = glm::ortho(
        -0.5f,
        static_cast<f32>(extent.width),
        -0.5f,
        static_cast<f32>(extent.height),
        -1.0f,
        1.0f
    );

    UniformBufferObject ubo{
        .ortho = ortho
    };

    m_ubo.update(&ubo, sizeof(ubo));

    int textWidth = 0;
    for (unsigned char c : text) {
        textWidth += m_charWidths[c];
    }

    int x = pos.x;
    int y = pos.y;

    switch (align)
    {
    case TextAlign::TOP_LEFT:
        x = pos.x;
        y = pos.y;
        break;

    case TextAlign::TOP_RIGHT:
        x = extent.width - textWidth - pos.x;
        y = pos.y;
        break;

    case TextAlign::BOTTOM_LEFT:
        x = pos.x;
        y = extent.height - size - pos.y;
        break;

    case TextAlign::BOTTOM_RIGHT:
        x = extent.width - textWidth - pos.x;
        y = extent.height - size - pos.y;
        break;

    case TextAlign::CENTER:
        x = (extent.width / 2 - textWidth - size / 2) + pos.x;
        y = (extent.height / 2 - size / 2) + pos.y;
        break;
    }
        

    x = std::floor(x);

    const f32 pixelOffset = static_cast<float>(size) / 8.0f;

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

        m_pipeline.push(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
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
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
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