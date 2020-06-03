#include "Sampler.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::Sampler::CreateInfo::CreateInfo()
{
    m_VkSamplerCreateInfo = {};
    m_VkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
}

krt::Sampler::CreateInfo krt::Sampler::CreateInfo::CreateDefault()
{
    CreateInfo info = {};
    info->addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info->addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info->addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info->anisotropyEnable = VK_TRUE;
    info->maxAnisotropy = 16.0f;
    info->borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    info->unnormalizedCoordinates = VK_FALSE;
    info->compareEnable = VK_FALSE;
    info->compareOp = VK_COMPARE_OP_ALWAYS;
    info->magFilter = VK_FILTER_LINEAR;
    info->minFilter = VK_FILTER_LINEAR;
    info->maxLod = 0.0f;
    info->minLod = 0.0f;
    info->mipLodBias = 0.0f;
    info->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    return info;
}

VkSamplerCreateInfo* krt::Sampler::CreateInfo::operator->()
{
    return &m_VkSamplerCreateInfo;
}

VkSamplerCreateInfo* krt::Sampler::CreateInfo::operator&()
{
    return &m_VkSamplerCreateInfo;
}

krt::Sampler::Sampler(ServiceLocator& a_Services, CreateInfo a_CreateInfo)
    : m_Services(a_Services)
{
    vkCreateSampler(m_Services.m_LogicalDevice->GetVkDevice(), &a_CreateInfo, m_Services.m_AllocationCallbacks, &m_VkSampler);
}

krt::Sampler::~Sampler()
{
    vkDestroySampler(m_Services.m_LogicalDevice->GetVkDevice(), m_VkSampler, m_Services.m_AllocationCallbacks);
}
