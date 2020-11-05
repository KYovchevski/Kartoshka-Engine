#include "DepthBuffer.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "CommandQueue.h"

krt::DepthBuffer::DepthBuffer(ServiceLocator& a_Services, uint32_t a_Width, uint32_t a_Height,
    VkFormat a_Format, std::set<ECommandQueueType>& a_UsingQueues)
    : Texture(a_Services, a_Format)
{
    std::set<uint32_t> uniqueIndices;
    for (auto& queueType : a_UsingQueues)
        uniqueIndices.insert(m_Services.m_LogicalDevice->GetCommandQueue(queueType).GetFamilyIndex());

    std::vector<uint32_t> queueIndices(uniqueIndices.begin(), uniqueIndices.end());

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.format = m_Format;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = 1;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.extent.width = a_Width;
    imageInfo.extent.height = a_Height;
    imageInfo.extent.depth = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.pQueueFamilyIndices = queueIndices.data();
    imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueIndices.size());
    imageInfo.sharingMode = queueIndices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    
    vkCreateImage(m_Services.m_LogicalDevice->GetVkDevice(), &imageInfo, m_Services.m_AllocationCallbacks, &m_VkImage);

    auto memoryInfo = m_Services.m_PhysicalDevice->GetMemoryInfoForImage(m_VkImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = memoryInfo.m_MemoryType;
    allocInfo.allocationSize = memoryInfo.m_Size;

    vkAllocateMemory(m_Services.m_LogicalDevice->GetVkDevice(), &allocInfo, m_Services.m_AllocationCallbacks, &m_VkDeviceMemory);
    vkBindImageMemory(m_Services.m_LogicalDevice->GetVkDevice(), m_VkImage, m_VkDeviceMemory, 0);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = m_VkImage;
    viewInfo.format = m_Format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;

    vkCreateImageView(m_Services.m_LogicalDevice->GetVkDevice(), &viewInfo, m_Services.m_AllocationCallbacks, &m_VkImageView);
}

krt::DepthBuffer::~DepthBuffer()
{
}
