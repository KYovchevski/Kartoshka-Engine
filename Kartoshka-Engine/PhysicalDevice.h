#pragma once

#include "vulkan/vulkan.h"


#include <optional>
#include <set>
#include <vector>

namespace krt
{
    struct ServiceLocator;
}

namespace krt
{
    struct QueueFamilyIndices final
    {
        std::optional<uint32_t> m_GraphicsQueueIndex;
        std::optional<uint32_t> m_ComputeQueueIndex;
        std::optional<uint32_t> m_PresentQueueIndex;
        std::optional<uint32_t> m_TransferQueueIndex;

        bool IsComplete() const;
        std::set<uint32_t> GetUniqueIndices() const;
    };


    class PhysicalDevice final
    {
    public:
        PhysicalDevice(ServiceLocator& a_Services, VkSurfaceKHR a_TargetSurface);
        ~PhysicalDevice();

        PhysicalDevice(PhysicalDevice&) = delete; // No copy c-tor
        PhysicalDevice(PhysicalDevice&&) = delete; // No move c-tor
        PhysicalDevice& operator=(PhysicalDevice&) = delete; // No copy assignment
        PhysicalDevice& operator=(PhysicalDevice&&) = delete; // No move assignment

        const QueueFamilyIndices& GetQueueFamilyIndices() const;
        VkPhysicalDevice GetPhysicalDevice();

        struct MemoryInfo
        {
            uint32_t m_MemoryType;
            VkDeviceSize m_Size;
            VkDeviceSize m_Alignment;
        };
        MemoryInfo GetMemoryInfoForBuffer(VkBuffer a_Buffer, VkMemoryPropertyFlags a_MemProperties);
        MemoryInfo GetMemoryInfoForImage(VkImage a_Image, VkMemoryPropertyFlags a_MemProperties);

    private:
        uint32_t FindMemoryType(uint32_t a_MemoryType, VkMemoryPropertyFlags a_Properties);
        static bool IsDeviceSuitable(VkPhysicalDevice a_PhysicalDevice, VkSurfaceKHR a_TargetSurface, std::vector<const char*>& a_ReqExtensions);
        static QueueFamilyIndices GetQueueFamilyIndicesForDevice(VkPhysicalDevice a_Device, VkSurfaceKHR a_TargetSurface);
        static bool CheckExtensionSupportForDevice(VkPhysicalDevice a_Device, std::vector<const char*>& a_ReqExtensions);

        ServiceLocator& m_Services;

        VkPhysicalDevice m_VkPhysicalDevice;
        QueueFamilyIndices m_QueueFamilyIndices;
    };
}
