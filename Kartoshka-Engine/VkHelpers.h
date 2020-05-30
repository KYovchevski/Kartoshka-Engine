#pragma once

#include "vulkan/vulkan.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

namespace krt
{
    // Shorthand to check if a VkResult is VK_SUCCESS and notify the user if it isn't
    void ThrowIfFailed(VkResult a_Result);

    // Namespace containing functions that help with the setup of Vulkan, mainly feature checks
    namespace hlp
    {
        struct SwapChainSupportDetails
        {
            VkSurfaceCapabilitiesKHR            m_SurfaceCapabilities;
            std::vector<VkSurfaceFormatKHR>     m_SurfaceFormats;
            std::vector<VkPresentModeKHR>       m_PresentModes;
        };

        SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice a_PhysicalDevice, VkSurfaceKHR a_Surface);
        VkSurfaceFormatKHR      ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& a_AvailableFormats);
        VkPresentModeKHR        ChoosePresentMode(const std::vector<VkPresentModeKHR>& a_AvailablePresentModes);
        VkExtent2D              ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& a_SurfaceCapabilities, const uint32_t a_RequestedWidth,
            const uint32_t a_RequestedHeight);

        VkShaderModule          CreateShaderModule(VkDevice a_Device, std::vector<char>& a_ShaderCode);

        uint32_t                FindMemoryType(const VkPhysicalDevice& a_PhysicalDevice, uint32_t a_TypeFilter, VkMemoryPropertyFlags a_MemoryProperties);

        std::vector<char>       LoadFile(const std::string& a_Filename);
    }

    namespace vkext
    {
        VkResult CreateDebugUtilsMessengerEXT(VkInstance a_Instance, VkDebugUtilsMessengerCreateInfoEXT& a_Info,
            VkAllocationCallbacks* a_AllocationCallbacks, VkDebugUtilsMessengerEXT& a_Messenger);
        

        VkResult DestroyDebugUtilsMessengerEXT(VkInstance a_Instance, VkDebugUtilsMessengerEXT& a_Messenger,
            VkAllocationCallbacks* a_Callbacks);
    }
}
