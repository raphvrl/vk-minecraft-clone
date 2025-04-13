#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <stdexcept>

#include "core/types.hpp"

namespace gfx::vk
{

void check(VkResult result, const std::string &msg);

} // namespace gfx::vk