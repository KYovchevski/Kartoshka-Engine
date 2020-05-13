#include "VkHelpers.h"



#include <fstream>
#include <algorithm>

void krt::ThrowIfFailed(enum VkResult a_Result)
{
    a_Result;
    // Don't want these checks running in release, because they shouldn't be needed
#ifdef _DEBUG
    if (a_Result != VK_SUCCESS)
    {
        printf("Vulkan error: %d", a_Result);
        abort();
    }
#endif
}

krt::hlp::SwapChainSupportDetails krt::hlp::QuerySwapChainSupportDetails(VkPhysicalDevice& a_PhysicalDevice, VkSurfaceKHR& a_Surface)
{
    SwapChainSupportDetails details;

    ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(a_PhysicalDevice, a_Surface, &details.m_SurfaceCapabilities));

    uint32_t surfaceFormatsCount;
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(a_PhysicalDevice, a_Surface, &surfaceFormatsCount, nullptr));
    details.m_SurfaceFormats.resize(surfaceFormatsCount);
    ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(a_PhysicalDevice, a_Surface, &surfaceFormatsCount, details.m_SurfaceFormats.data()));

    uint32_t presentModesCount;
    ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(a_PhysicalDevice, a_Surface, &presentModesCount, nullptr));
    details.m_PresentModes.resize(presentModesCount);
    ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(a_PhysicalDevice, a_Surface, &presentModesCount, details.m_PresentModes.data()));

    return details;
}

VkSurfaceFormatKHR krt::hlp::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& a_AvailableFormats)
{
    for (auto& availableFormat : a_AvailableFormats)
    {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return a_AvailableFormats[0];
}

VkPresentModeKHR krt::hlp::ChoosePresentMode(const std::vector<VkPresentModeKHR>& a_AvailablePresentModes)
{
    for (auto& availablePresentMode : a_AvailablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D krt::hlp::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& a_SurfaceCapabilities,const uint32_t a_RequestedWidth,
    const uint32_t a_RequestedHeight)
{
    if (a_SurfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
        return a_SurfaceCapabilities.currentExtent;

    VkExtent2D extent{ a_RequestedWidth, a_RequestedHeight };

    extent.width = std::clamp(extent.width, a_SurfaceCapabilities.minImageExtent.width, a_SurfaceCapabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, a_SurfaceCapabilities.minImageExtent.height, a_SurfaceCapabilities.maxImageExtent.height);

    return extent;
}

VkShaderModule krt::hlp::CreateShaderModule(VkDevice& a_Device, std::vector<char> a_ShaderCode)
{
    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = a_ShaderCode.size();
    moduleInfo.pCode = reinterpret_cast<uint32_t*>(a_ShaderCode.data());

    VkShaderModule shaderModule;

    ThrowIfFailed(vkCreateShaderModule(a_Device, &moduleInfo, nullptr, &shaderModule));

    return shaderModule;
}

uint32_t krt::hlp::FindMemoryType(const VkPhysicalDevice& a_PhysicalDevice, uint32_t a_TypeFilter,
    VkMemoryPropertyFlags a_MemoryProperties)
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemory;

    vkGetPhysicalDeviceMemoryProperties(a_PhysicalDevice, &physicalDeviceMemory);

    for (uint32_t i = 0; i < physicalDeviceMemory.memoryTypeCount; i++)
    {
        if (a_TypeFilter & (1 << i) && (physicalDeviceMemory.memoryTypes[i].propertyFlags & a_MemoryProperties) == a_MemoryProperties)
        {
            return i;
        }
    }

    printf("ERROR: Could not find suitable memory types.");
    abort();
}

std::vector<char> krt::hlp::LoadFile(const std::string& a_Filename)
{
    std::ifstream fstream(a_Filename, std::ios::ate | std::ios::binary);

    if (!fstream.is_open())
    {
        printf("Error: Could not open file: %s\n", a_Filename.c_str());
        abort();
    }

    auto fileSize = fstream.tellg();
    std::vector<char> output(fileSize);
    fstream.seekg(0);
    fstream.read(output.data(), fileSize);
    fstream.close();
    return output;
}

VkResult krt::vkext::CreateDebugUtilsMessengerEXT(VkInstance& a_Instance, VkDebugUtilsMessengerCreateInfoEXT& a_Info,
    VkAllocationCallbacks* a_AllocationCallbacks, VkDebugUtilsMessengerEXT& a_Messenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(a_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(a_Instance, &a_Info, a_AllocationCallbacks, &a_Messenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    return VK_SUCCESS;
}

VkResult krt::vkext::DestroyDebugUtilsMessengerEXT(VkInstance& a_Instance, VkDebugUtilsMessengerEXT& a_Messenger, VkAllocationCallbacks* a_Callbacks)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(a_Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(a_Instance, a_Messenger, a_Callbacks);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    return VK_SUCCESS;
}
