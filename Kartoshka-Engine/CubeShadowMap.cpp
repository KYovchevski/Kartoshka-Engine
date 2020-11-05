#include "CubeShadowMap.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"

#include <vector>

krt::CubeShadowMap::CubeShadowMap(ServiceLocator& a_ServiceLocator, uint32_t a_Dimensions, VkFormat a_ImageFormat)
    : Texture(a_ServiceLocator, a_ImageFormat)
    , m_Dimensions(a_Dimensions)
{
    CreateImage();
    CreateShaderImageView();
    CreateDepthImageViews();
}

std::array<VkImageView, 6> krt::CubeShadowMap::GetDepthViews()
{
    return m_DepthImageViews;
}

void krt::CubeShadowMap::TransitionLayoutToShaderRead(VkImageLayout a_OldLayout, VkAccessFlags a_AccessMask)
{
    VkImageMemoryBarrier barr = {};
    barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barr.image = m_VkImage;
    barr.srcAccessMask = a_AccessMask;
    barr.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barr.oldLayout = a_OldLayout;
    barr.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barr.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barr.subresourceRange.baseArrayLayer = 0;
    barr.subresourceRange.layerCount = 6;
    barr.subresourceRange.baseMipLevel = 0;
    barr.subresourceRange.levelCount = 1;
    barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    auto& cmd = m_Services.m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetSingleUseCommandBuffer();
    cmd.Begin();
    vkCmdPipelineBarrier(cmd.GetVkCommandBuffer(), VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
        0, nullptr, 1, &barr);
    cmd.End();
    cmd.Submit();
    //m_Services.m_LogicalDevice->GetCommandQueue(EGraphicsQueue).Flush();
}

void krt::CubeShadowMap::CreateImage()
{
    std::vector<uint32_t> familyIndices;

    familyIndices.push_back(m_Services.m_LogicalDevice->GetQueueIndices({ EGraphicsQueue })[0]);

    VkImageCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.format = m_Format;
    info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent.width = m_Dimensions;
    info.extent.height = m_Dimensions;
    info.extent.depth = 1;
    info.arrayLayers = 6;
    info.mipLevels = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.pQueueFamilyIndices = familyIndices.data();
    info.queueFamilyIndexCount = static_cast<uint32_t>(familyIndices.size());
    info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    vkCreateImage(m_Services.m_LogicalDevice->GetVkDevice(), &info, m_Services.m_AllocationCallbacks, &m_VkImage);

    auto memInfo = m_Services.m_PhysicalDevice->GetMemoryInfoForImage(m_VkImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memInfo.m_Size;
    allocInfo.memoryTypeIndex = memInfo.m_MemoryType;

    vkAllocateMemory(m_Services.m_LogicalDevice->GetVkDevice(), &allocInfo, m_Services.m_AllocationCallbacks, &m_VkDeviceMemory);

    vkBindImageMemory(m_Services.m_LogicalDevice->GetVkDevice(), m_VkImage, m_VkDeviceMemory, 0);

}

void krt::CubeShadowMap::CreateShaderImageView()
{
    VkImageViewCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    info.format = m_Format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    info.image = m_VkImage;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 6;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;

    vkCreateImageView(m_Services.m_LogicalDevice->GetVkDevice(), &info, m_Services.m_AllocationCallbacks, &m_VkImageView);
}

void krt::CubeShadowMap::CreateDepthImageViews()
{
    VkImageViewCreateInfo info = {};

    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    info.format = m_Format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    info.image = m_VkImage;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    for (auto& depthImageView : m_DepthImageViews)
    {
        vkCreateImageView(m_Services.m_LogicalDevice->GetVkDevice(), &info, m_Services.m_AllocationCallbacks, &depthImageView);
        info.subresourceRange.baseArrayLayer++;
    }
}
