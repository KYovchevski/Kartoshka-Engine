#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "VkHelpers.h"
#include "VkConstants.h"

#include "ServiceLocator.h"

bool krt::QueueFamilyIndices::IsComplete() const
{
    return m_ComputeQueueIndex.has_value() && m_PresentQueueIndex.has_value() && 
        m_GraphicsQueueIndex.has_value() && m_TransferQueueIndex.has_value();
}

std::set<uint32_t> krt::QueueFamilyIndices::GetUniqueIndices() const
{
    return std::set<uint32_t> {m_ComputeQueueIndex.value(), m_PresentQueueIndex.value(),
        m_GraphicsQueueIndex.value(), m_TransferQueueIndex.value()};
}

krt::PhysicalDevice::PhysicalDevice(ServiceLocator& a_Services, VkSurfaceKHR a_TargetSurface)
    : m_Services(a_Services)
{
    printf("Picking best available physical device. \n");
    // Find the number of physical devices in the system that support vulkan
    uint32_t deviceCount;
    ThrowIfFailed(vkEnumeratePhysicalDevices(m_Services.m_LogicalDevice->GetVkInstance(), &deviceCount, nullptr));

    if (!deviceCount)
    {
        printf("ERROR: Could not find any GPUs with support for Vulkan. Ending execution.\n");
        abort();
    }

    // Queue the actual physical devices
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    ThrowIfFailed(vkEnumeratePhysicalDevices(m_Services.m_LogicalDevice->GetVkInstance(), &deviceCount, physicalDevices.data()));

    //auto reqExtensions = m_Services.m_LogicalDevice->GetRequiredExtensions();
    auto reqExtensions = constants::VkDeviceExtensions;

    // Find the first suitable physical device
    for (auto& dev : physicalDevices)
    {
        if (IsDeviceSuitable(dev, a_TargetSurface, reqExtensions))
        {
            m_VkPhysicalDevice = dev;
            m_QueueFamilyIndices = GetQueueFamilyIndicesForDevice(m_VkPhysicalDevice, a_TargetSurface);
            break;
        }
    }

    if (m_VkPhysicalDevice == VK_NULL_HANDLE)
    {
        printf("ERROR: Could not find suitable GPUs in the system. Ending execution.\n");
        abort();
    }
}

krt::PhysicalDevice::~PhysicalDevice()
{
    
}

const krt::QueueFamilyIndices& krt::PhysicalDevice::GetQueueFamilyIndices() const
{
    return m_QueueFamilyIndices;
}

VkPhysicalDevice krt::PhysicalDevice::GetPhysicalDevice()
{
    return m_VkPhysicalDevice;
}

uint32_t krt::PhysicalDevice::FindMemoryType(VkBuffer a_TargetBuffer, VkMemoryPropertyFlags a_Properties)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Services.m_LogicalDevice->GetVkDevice(), a_TargetBuffer, &memRequirements);

    return hlp::FindMemoryType(m_VkPhysicalDevice, memRequirements.memoryTypeBits, a_Properties);
}

bool krt::PhysicalDevice::IsDeviceSuitable(VkPhysicalDevice a_PhysicalDevice, VkSurfaceKHR a_TargetSurface, std::vector<const char*>& a_ReqExtensions)
{
    const auto indices = GetQueueFamilyIndicesForDevice(a_PhysicalDevice, a_TargetSurface);


    const bool extensionsSupported = CheckExtensionSupportForDevice(a_PhysicalDevice, a_ReqExtensions);

    const auto swapChainDetails = hlp::QuerySwapChainSupportDetails(a_PhysicalDevice, a_TargetSurface);
    const bool swapChainSupported = !swapChainDetails.m_SurfaceFormats.empty() && !swapChainDetails.m_PresentModes.empty();


    return indices.IsComplete() && extensionsSupported && swapChainSupported;
}

krt::QueueFamilyIndices krt::PhysicalDevice::GetQueueFamilyIndicesForDevice(VkPhysicalDevice a_Device, VkSurfaceKHR a_TargetSurface)
{
    QueueFamilyIndices indices;
    uint32_t familiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(a_Device, &familiesCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(a_Device, &familiesCount, families.data());

    for (size_t i = 0; i < families.size(); i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !indices.m_GraphicsQueueIndex.has_value())
            indices.m_GraphicsQueueIndex = static_cast<uint32_t>(i);

        if (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !indices.m_ComputeQueueIndex.has_value())
            indices.m_ComputeQueueIndex = static_cast<uint32_t>(i);

        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(a_Device, static_cast<uint32_t>(i), a_TargetSurface, &presentSupport);
        if (presentSupport)
            indices.m_PresentQueueIndex = static_cast<uint32_t>(i);

        if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
            indices.m_TransferQueueIndex = static_cast<uint32_t>(i);

        if (indices.IsComplete())
            break;
    }

    return indices;
}

bool krt::PhysicalDevice::CheckExtensionSupportForDevice(VkPhysicalDevice a_Device, std::vector<const char*>& a_ReqExtensions)
{
    uint32_t extCount;
    vkEnumerateDeviceExtensionProperties(a_Device, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extCount);
    vkEnumerateDeviceExtensionProperties(a_Device, nullptr, &extCount, availableExtensions.data());

    auto required = std::vector<const char*>(a_ReqExtensions.begin(), a_ReqExtensions.end());
    std::vector<const char*> unsupported;

    for (auto& req : required)
    {
        bool found = false;
        for (auto& ext : availableExtensions)
        {
            if (strcmp(ext.extensionName, req))
            {
                found = true;
            }
        }
        if (!found)
            unsupported.push_back(req);
    }

    return unsupported.empty();
}

