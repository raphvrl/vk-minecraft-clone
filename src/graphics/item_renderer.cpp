#include "item_renderer.hpp"

namespace gfx
{

void ItemRenderer::init(Device &device, TextureCache &textureCache)
{
    m_device = &device;

    m_pipeline = Pipeline::Builder(device)
        .setShader("item.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("item.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setPushConstant(sizeof(PushConstant))
        .setDepthTest(true)
        .setDepthWrite(true)
        .build();

    m_textureID = textureCache.getTextureID("terrain");
}

void ItemRenderer::destroy()
{
    m_pipeline.destroy();
}

void ItemRenderer::render(
    VkCommandBuffer cmd,
    wld::BlockType blockType,
    const glm::vec2 &position,
    f32 scale
)
{
    auto &block = wld::BlockRegistry::get().getBlock(blockType);
    auto &textures = block.textures;

    PushConstant pc;
    
    pc.topUV = {
        textures.getUV(wld::Face::TOP).x,
        textures.getUV(wld::Face::TOP).y
    };
    pc.bottomUV = {
        textures.getUV(wld::Face::BOTTOM).x,
        textures.getUV(wld::Face::BOTTOM).y
    };
    pc.sideUV = {
        textures.getUV(wld::Face::NORTH).x,
        textures.getUV(wld::Face::NORTH).y
    };
    
    pc.textureID = m_textureID;
    pc.model = glm::mat4(1.0f);
    pc.model = glm::translate(pc.model, {position.x, position.y, -scale});
    pc.model = glm::rotate(pc.model, glm::radians(-30.0f), {1.0f, 0.0f, 0.0f});
    pc.model = glm::rotate(pc.model, glm::radians(45.0f), {0.0f, 1.0f, 0.0f});
    pc.model = glm::scale(pc.model, {scale, scale, scale});

    m_pipeline.bind(cmd);
    m_pipeline.push(cmd, pc);

    gfx::drawCube(cmd);
}

} // namespace gfx