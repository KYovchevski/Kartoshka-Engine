#pragma once
/*
 *
 * File with the purpose of storing all constants related to Vulkan as a mean of easier access.
 * This currently includes the following constants:
 *  -- The validation layers to use when running in debug
 *  -- The required extensions
 *
 */


#include <vulkan/vulkan_core.h>

#include <vector>

namespace krt
{
    namespace constants
    {
        const std::vector<const char*> VkValidationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> VkDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
    }

}
