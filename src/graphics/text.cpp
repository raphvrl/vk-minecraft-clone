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

    std::vector<DescriptorData> descriptors = {
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

    m_charWidths.fill(4);

    m_charWidths['a'] = 6;
    m_charWidths['b'] = 6;
    m_charWidths['c'] = 5;
    m_charWidths['d'] = 6;
    m_charWidths['e'] = 6;
    m_charWidths['f'] = 5;
    m_charWidths['g'] = 6;
    m_charWidths['h'] = 6;
    m_charWidths['i'] = 2;
    m_charWidths['j'] = 5;
    m_charWidths['k'] = 6;
    m_charWidths['l'] = 2;
    m_charWidths['m'] = 8;
    m_charWidths['n'] = 6;
    m_charWidths['o'] = 6;
    m_charWidths['p'] = 6;
    m_charWidths['q'] = 6;
    m_charWidths['r'] = 5;
    m_charWidths['s'] = 5;
    m_charWidths['t'] = 5;
    m_charWidths['u'] = 6;
    m_charWidths['v'] = 6;
    m_charWidths['w'] = 8;
    m_charWidths['x'] = 6;
    m_charWidths['y'] = 6;
    m_charWidths['z'] = 6;

    m_charWidths['A'] = 7;
    m_charWidths['B'] = 7;
    m_charWidths['C'] = 7;
    m_charWidths['D'] = 7;
    m_charWidths['E'] = 6;
    m_charWidths['F'] = 6;
    m_charWidths['G'] = 7;
    m_charWidths['H'] = 7;
    m_charWidths['I'] = 3;
    m_charWidths['J'] = 6;
    m_charWidths['K'] = 7;
    m_charWidths['L'] = 6;
    m_charWidths['M'] = 9;
    m_charWidths['N'] = 7;
    m_charWidths['O'] = 7;
    m_charWidths['P'] = 6;
    m_charWidths['Q'] = 7;
    m_charWidths['R'] = 7;
    m_charWidths['S'] = 6;
    m_charWidths['T'] = 6;
    m_charWidths['U'] = 7;
    m_charWidths['V'] = 7;
    m_charWidths['W'] = 9;
    m_charWidths['X'] = 7;
    m_charWidths['Y'] = 7;
    m_charWidths['Z'] = 7;

    m_charWidths['0'] = 6;
    m_charWidths['1'] = 6;
    m_charWidths['2'] = 6;
    m_charWidths['3'] = 6;
    m_charWidths['4'] = 6;
    m_charWidths['5'] = 6;
    m_charWidths['6'] = 6;
    m_charWidths['7'] = 6;
    m_charWidths['8'] = 6;
    m_charWidths['9'] = 6;

    m_charWidths[' '] = 3;
    m_charWidths['.'] = 2;
    m_charWidths[','] = 2;
    m_charWidths['!'] = 2;
    m_charWidths['?'] = 6;
    m_charWidths['/'] = 6;
    m_charWidths['\\'] = 6;
    m_charWidths['('] = 4;
    m_charWidths[')'] = 4;
    m_charWidths['['] = 4;
    m_charWidths[']'] = 4;
    m_charWidths['{'] = 4;
    m_charWidths['}'] = 4;
    m_charWidths[':'] = 2;
    m_charWidths[';'] = 2;
    m_charWidths['+'] = 6;
    m_charWidths['-'] = 6;
    m_charWidths['_'] = 6;
    m_charWidths['='] = 6;
    m_charWidths['*'] = 5;
    m_charWidths['&'] = 7;
    m_charWidths['^'] = 5;
    m_charWidths['%'] = 7;
    m_charWidths['$'] = 6;
    m_charWidths['#'] = 7;
    m_charWidths['@'] = 7;
    m_charWidths['~'] = 6;
    m_charWidths['`'] = 3;
    m_charWidths['\''] = 2;
    m_charWidths['\"'] = 5;
    m_charWidths['<'] = 5;
    m_charWidths['>'] = 5;
}

void Text::destroy()
{
    m_ubo.destroy();
    m_font.destroy();
    m_pipeline.destroy();
}

void Text::draw(
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

    glm::mat4 proj = glm::ortho(
        -0.5f,
        static_cast<f32>(extent.width),
        -0.5f,
        static_cast<f32>(extent.height),
        -1.0f,
        1.0f
    );

    UniformBufferObject ubo{
        .proj = proj
    };

    m_ubo.update(&ubo, sizeof(ubo));

    int textWidth = 0;
    for (unsigned char c : text) {
        textWidth += m_charWidths[c];
    }

    int x = pos.x;

    if (align == TextAlign::CENTER) {
        x -= textWidth / 2;
    } else if (align == TextAlign::RIGHT) {
        x -= textWidth;
    }

    x = std::floor(x);

    f32 pixelOffset = static_cast<float>(size) / 8.0f;

    for (unsigned char c : text) {
        int col = c % 16;
        int row = c / 16;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(
            x + pixelOffset,
            pos.y + pixelOffset,
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
        model = glm::translate(model, glm::vec3(x, pos.y, 0.0f));
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

} // namespace gfx