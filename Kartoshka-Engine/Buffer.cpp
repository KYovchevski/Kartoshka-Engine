#include "Buffer.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::Buffer::Buffer(ServiceLocator& a_Services)
    : m_Services(a_Services)
    , m_VkBuffer(VK_NULL_HANDLE)
    , m_VkDeviceMemory(VK_NULL_HANDLE)
{
}

krt::Buffer::~Buffer()
{
    vkDestroyBuffer(m_Services.m_LogicalDevice->GetVkDevice(), m_VkBuffer, m_Services.m_AllocationCallbacks);
    vkFreeMemory(m_Services.m_LogicalDevice->GetVkDevice(), m_VkDeviceMemory, m_Services.m_AllocationCallbacks);
}
