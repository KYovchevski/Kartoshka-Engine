#pragma once


#include "vulkan/vulkan.h"

#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace krt
{
    struct ServiceLocator;
    class CommandQueue;
    class PhysicalDevice;
    class Buffer;
}

namespace krt
{


    enum ECommandQueueType : uint8_t
    {
        EGraphicsQueue = VK_QUEUE_GRAPHICS_BIT, // 1
        EComputeQueue = VK_QUEUE_COMPUTE_BIT, // 2
        EPresentQueue = 3, // The present queue is established differently, so there is no VK_QUEUE enum for it
        ETransferQueue = VK_QUEUE_TRANSFER_BIT, // 4
    };

    class LogicalDevice final
    {
        friend krt::PhysicalDevice;
    public:
        LogicalDevice(ServiceLocator& a_Services, const std::string& a_WindowTitle, const std::string& a_EngineName);
        ~LogicalDevice();

        LogicalDevice(LogicalDevice&) = delete;             // No copy c-tor
        LogicalDevice(LogicalDevice&&) = delete;            // No move c-tor
        LogicalDevice& operator=(LogicalDevice&) = delete;  // No copy assignment operator
        LogicalDevice& operator=(LogicalDevice&&) = delete; // No move assignment operator

        void                InitializeDevice();

        // Gets the handle to the VkDevice. This is a null handle until InitializeDevice has been called.
        VkDevice            GetVkDevice() const;
        VkInstance          GetVkInstance() const;
        CommandQueue&       GetCommandQueue(const ECommandQueueType a_Type);

        template<typename BufferType = Buffer>
        std::unique_ptr<BufferType> CreateBuffer(uint64_t a_Size, VkBufferUsageFlags a_Usage,
            VkMemoryPropertyFlags a_MemoryProperties, const std::set<ECommandQueueType>& a_QueuesWithAccess);

        // Resize an existing Buffer object. Does not preserve the current buffer content by default.
        // If the buffer is being resized to a smaller size, the contents are never preserved.
        void ResizeBuffer(Buffer& a_Buffer, uint64_t a_NewSize, bool a_PreserveContent = false);

        void CopyToDeviceMemory(VkDeviceMemory a_DeviceMemory, const void* a_Data, uint64_t a_DataSize);

        std::vector<uint32_t> GetQueueIndices(const std::set<ECommandQueueType>& a_Queues);

        // Flushes all command queues to ensure that the device is idle
        void Flush();
    private:

        std::pair<VkBuffer, VkDeviceMemory> CreateBufferElements(uint64_t a_Size, VkBufferUsageFlags a_Usage,
            VkMemoryPropertyFlags a_MemoryProperties, const std::set<ECommandQueueType>& a_QueuesWithAccess);
        std::vector<const char*>    GetRequiredExtensions() const;
        bool                        CheckValidationLayerSupport() const;
        bool                        ValidateExtensionSupport(std::vector<const char*> a_Extensions) const;

        std::vector<VkDeviceQueueCreateInfo> GenerateQueueInfos();

    private:

        ServiceLocator& m_Services;

        VkInstance                      m_VkInstance;
        VkDevice                        m_VkLogicalDevice;

        std::map<ECommandQueueType, std::unique_ptr<CommandQueue>> m_CommandQueues;
    };

}

#include "LogicalDevice.inl"