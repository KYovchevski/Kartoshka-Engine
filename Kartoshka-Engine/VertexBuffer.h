#pragma once

#include "Buffer.h"

#include "vulkan/vulkan.h"

namespace krt
{
    struct ServiceLocator;
    class CommandBuffer;
}

namespace krt
{

    class VertexBuffer : public Buffer
    {
        friend CommandBuffer;
    public:
        VertexBuffer(ServiceLocator& a_Services, uint64_t a_InitialSize, VkBufferUsageFlags a_UsageFlags,
            VkMemoryPropertyFlags a_MemoryPropertyFlags, std::set<ECommandQueueType>  a_QueuesWithAccess);
        ~VertexBuffer();

        uint32_t GetElementCount();
        VkBuffer GetVkBuffer() const { return m_VkBuffer; }

    private:

        uint32_t m_NumElements;
        
    };

}