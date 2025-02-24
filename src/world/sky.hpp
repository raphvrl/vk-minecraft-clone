#pragma once

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"

namespace wld
{

class Sky
{

public:
    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void render();

private:
    gfx::VulkanCtx *m_ctx;

    gfx::Pipeline m_pipeline;

};

} // namespace wld