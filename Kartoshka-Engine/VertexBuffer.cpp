#include "VertexBuffer.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::VertexBuffer::VertexBuffer(ServiceLocator& a_Services, uint64_t a_InitialSize, VkBufferUsageFlags a_UsageFlags,
    VkMemoryPropertyFlags a_MemoryPropertyFlags, std::set<ECommandQueueType>  a_QueuesWithAccess)
    : Buffer(a_Services, a_InitialSize, a_UsageFlags, a_MemoryPropertyFlags, a_QueuesWithAccess)
{
    
}


krt::VertexBuffer::~VertexBuffer()
{
    
}

uint32_t krt::VertexBuffer::GetElementCount()
{
    return m_NumElements;
}
