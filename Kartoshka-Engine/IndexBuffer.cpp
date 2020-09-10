#include "IndexBuffer.h"

krt::IndexBuffer::IndexBuffer(ServiceLocator& a_Services, uint64_t a_InitialSize, VkBufferUsageFlags a_UsageFlags,
    VkMemoryPropertyFlags a_MemoryPropertyFlags, std::set<ECommandQueueType>  a_QueuesWithAccess)
    : Buffer(a_Services, a_InitialSize, a_UsageFlags, a_MemoryPropertyFlags, a_QueuesWithAccess)
{

}

krt::IndexBuffer::~IndexBuffer()
{
}
