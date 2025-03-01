#include "text.hpp"

namespace gfx
{

void Text::init(VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = Pipeline::Builder(ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "text.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "text.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
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

    m_font.init(*m_ctx, "font.png");

    m_descriptorSet = m_pipeline.createDescriptorSet({
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .texture = &m_font
        }
    });

    m_charWidths.fill(0.8f);

    m_charWidths[' '] = 0.4f;

    m_charWidths['i'] = 0.4f;
    m_charWidths['l'] = 0.4f;
    m_charWidths['.'] = 0.3f;
    m_charWidths[','] = 0.3f;
    m_charWidths['\''] = 0.3f;
    m_charWidths['!'] = 0.3f;
    m_charWidths[':'] = 0.3f;
    m_charWidths[';'] = 0.3f;
    m_charWidths['|'] = 0.3f;
    m_charWidths['('] = 0.5f;
    m_charWidths[')'] = 0.5f;
    m_charWidths['['] = 0.5f;
    m_charWidths[']'] = 0.5f;
    m_charWidths['t'] = 0.7f;
    m_charWidths['f'] = 0.7f;
    m_charWidths['j'] = 0.6f;

    m_charWidths['m'] = 1.1f;
    m_charWidths['w'] = 1.1f;
    m_charWidths['M'] = 1.1f;
}

void Text::destroy()
{
    m_font.destroy();
    m_pipeline.destroy();
}

void Text::draw(
    const std::string &text,
    const glm::vec2 &pos,
    f32 size
)
{
    VkCommandBuffer cmd = m_ctx->getCommandBuffer();

    m_pipeline.bind();
    m_pipeline.bindDescriptorSet(m_descriptorSet);

    f32 x = pos.x;

    for (unsigned char c : text) {
        int col = c % 16;
        int row = c / 16;

        PushConstant pc;
        pc.pos = glm::vec2(x, pos.y);
        pc.uv = glm::vec2(
            static_cast<float>(col) / 16.0f,
            static_cast<float>(row) / 16.0f
        );
        pc.size = size;

        m_pipeline.push(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(pc),
            &pc
        );

        vkCmdDraw(cmd, 6, 1, 0, 0);

        x += size * m_charWidths[c];
    }
}

} // namespace gfx