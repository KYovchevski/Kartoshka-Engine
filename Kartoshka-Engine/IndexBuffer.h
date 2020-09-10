#pragma once

#include "Buffer.h"

namespace krt
{
    class IndexBuffer : public Buffer
    {
        friend class CommandBuffer;
    public:
        IndexBuffer(ServiceLocator& a_Services, uint64_t a_InitialSize, VkBufferUsageFlags a_UsageFlags,
            VkMemoryPropertyFlags a_MemoryPropertyFlags, std::set<ECommandQueueType> a_QueuesWithAccess);
        ~IndexBuffer();

        uint32_t GetElementCount() const { return m_NumElements; }

    private:
        VkIndexType m_IndexType;
        uint32_t m_NumElements;

    };
}
