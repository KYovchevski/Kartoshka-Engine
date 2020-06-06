#include "VertexBuffer.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::VertexBuffer::VertexBuffer(ServiceLocator& a_Services)
    : Buffer(a_Services)
{
    
}


krt::VertexBuffer::~VertexBuffer()
{
    vkFreeMemory(m_Services.m_LogicalDevice->GetVkDevice(), m_VkDeviceMemory, m_Services.m_AllocationCallbacks);
    vkDestroyBuffer(m_Services.m_LogicalDevice->GetVkDevice(), m_VkBuffer, m_Services.m_AllocationCallbacks);
}

uint32_t krt::VertexBuffer::GetElementCount()
{
    return m_NumElements;
}
