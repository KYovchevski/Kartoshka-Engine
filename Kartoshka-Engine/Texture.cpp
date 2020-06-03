#include "Texture.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::Texture::~Texture()
{
    vkDestroyImage(m_Services.m_LogicalDevice->GetVkDevice(), m_VkImage, m_Services.m_AllocationCallbacks);
    vkDestroyImageView(m_Services.m_LogicalDevice->GetVkDevice(), m_VkImageView, m_Services.m_AllocationCallbacks);
    vkFreeMemory(m_Services.m_LogicalDevice->GetVkDevice(), m_VkDeviceMemory, m_Services.m_AllocationCallbacks);
}

krt::Texture::Texture(ServiceLocator& a_Services)
    : m_Services(a_Services)
{

}
