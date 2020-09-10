#include "Buffer.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::Buffer::Buffer(ServiceLocator& a_Services, uint64_t a_InitialSize, VkBufferUsageFlags a_UsageFlags,
    VkMemoryPropertyFlags a_MemoryPropertyFlags, std::set<ECommandQueueType>  a_QueuesWithAccess)
    : m_Services(a_Services)
    , m_VkBuffer(VK_NULL_HANDLE)
    , m_VkDeviceMemory(VK_NULL_HANDLE)
    , m_BufferSize(a_InitialSize)
    , m_UsageFlags(a_UsageFlags)
    , m_MemoryPropertyFlags(a_MemoryPropertyFlags)
    , m_QueuesWithAccess(a_QueuesWithAccess)
{
}

krt::Buffer::~Buffer()
{
    vkDestroyBuffer(m_Services.m_LogicalDevice->GetVkDevice(), m_VkBuffer, m_Services.m_AllocationCallbacks);
    vkFreeMemory(m_Services.m_LogicalDevice->GetVkDevice(), m_VkDeviceMemory, m_Services.m_AllocationCallbacks);
}

