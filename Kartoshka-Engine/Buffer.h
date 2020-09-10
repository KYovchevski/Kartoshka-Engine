#pragma once

#include "vulkan/vulkan.h"

#include "set"

namespace krt
{
    struct ServiceLocator;
    enum ECommandQueueType : uint8_t;
}

namespace krt
{
    class Buffer
    {
    public:
        Buffer(ServiceLocator& a_Services, uint64_t a_InitialSize, VkBufferUsageFlags a_UsageFlags,
            VkMemoryPropertyFlags a_MemoryPropertyFlags, std::set<ECommandQueueType> a_QueuesWithAccess);
        virtual ~Buffer();


        VkBuffer m_VkBuffer;
        VkDeviceMemory m_VkDeviceMemory;
        const VkBufferUsageFlags m_UsageFlags;
        const VkMemoryPropertyFlags m_MemoryPropertyFlags;
        const std::set<ECommandQueueType> m_QueuesWithAccess;
        uint64_t m_BufferSize; // The current size of the Buffer object in bytes
    protected:
        ServiceLocator& m_Services;

    };
}
