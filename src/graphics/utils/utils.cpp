#include "utils.hpp"

namespace gfx::vk
{

void check(VkResult result, const std::string &msg)
{
    if (result == VK_SUCCESS) { return; }

    std::string errorMsg = msg + " - Error: ";
        
    switch (result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        errorMsg += "VK_ERROR_OUT_OF_HOST_MEMORY: A host memory allocation has failed.";
        break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        errorMsg += "VK_ERROR_OUT_OF_DEVICE_MEMORY: A device memory allocation has failed.";
        break;
        case VK_ERROR_INITIALIZATION_FAILED:
        errorMsg += "VK_ERROR_INITIALIZATION_FAILED: Initialization of an object could not be completed.";
        break;
        case VK_ERROR_DEVICE_LOST:
        errorMsg += "VK_ERROR_DEVICE_LOST: The logical or physical device has been lost.";
        break;
        case VK_ERROR_MEMORY_MAP_FAILED:
        errorMsg += "VK_ERROR_MEMORY_MAP_FAILED: Mapping of a memory object has failed.";
        break;
        case VK_ERROR_LAYER_NOT_PRESENT:
        errorMsg += "VK_ERROR_LAYER_NOT_PRESENT: A requested layer is not present or could not be loaded.";
        break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
        errorMsg += "VK_ERROR_EXTENSION_NOT_PRESENT: A requested extension is not supported.";
        break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
        errorMsg += "VK_ERROR_FEATURE_NOT_PRESENT: A requested feature is not supported.";
        break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
        errorMsg += "VK_ERROR_INCOMPATIBLE_DRIVER: The requested version of Vulkan is not supported by the driver.";
        break;
        case VK_ERROR_TOO_MANY_OBJECTS:
        errorMsg += "VK_ERROR_TOO_MANY_OBJECTS: Too many objects of the type have already been created.";
        break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
        errorMsg += "VK_ERROR_FORMAT_NOT_SUPPORTED: A requested format is not supported on this device.";
        break;
        case VK_ERROR_FRAGMENTED_POOL:
        errorMsg += "VK_ERROR_FRAGMENTED_POOL: A pool allocation has failed due to fragmentation of the pool's memory.";
        break;
        case VK_ERROR_SURFACE_LOST_KHR:
        errorMsg += "VK_ERROR_SURFACE_LOST_KHR: A surface is no longer available.";
        break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        errorMsg += "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: The requested window is already in use by Vulkan or another API.";
        break;
        case VK_ERROR_OUT_OF_DATE_KHR:
        errorMsg += "VK_ERROR_OUT_OF_DATE_KHR: A surface has changed in such a way that it is no longer compatible with the swapchain.";
        break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        errorMsg += "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: The display used by a swapchain does not use the same presentable image layout.";
        break;
        case VK_ERROR_VALIDATION_FAILED_EXT:
        errorMsg += "VK_ERROR_VALIDATION_FAILED_EXT: A validation layer found an error.";
        break;
        case VK_ERROR_INVALID_SHADER_NV:
        errorMsg += "VK_ERROR_INVALID_SHADER_NV: One or more shaders failed to compile or link.";
        break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        errorMsg += "VK_ERROR_INVALID_EXTERNAL_HANDLE: An external handle is not a valid handle of the specified type.";
        break;
        case VK_ERROR_FRAGMENTATION:
        errorMsg += "VK_ERROR_FRAGMENTATION: A descriptor pool creation has failed due to fragmentation.";
        break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        errorMsg += "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: A buffer creation failed because the requested address is not available.";
        break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        errorMsg += "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: An operation on a swapchain created with full screen exclusive mode failed as it did not have exclusive full-screen access.";
        break;
        default:
        errorMsg += "Unknown Vulkan error code: " + std::to_string(result);
        break;
    }

    throw std::runtime_error(errorMsg);
}

} // namespace gfx::vk