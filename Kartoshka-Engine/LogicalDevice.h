#pragma once


#include "vulkan/vulkan.h"

#include <memory>
#include <map>
#include <string>
#include <vector>

namespace krt
{
    struct ServiceLocator;
    class CommandQueue;
    class PhysicalDevice;
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


    private:

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
