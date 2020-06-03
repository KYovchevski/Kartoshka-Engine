#include "DescriptorSetPoolPage.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "DescriptorSetAllocation.h"

#include <cassert>

krt::DescriptorSetPoolPage::DescriptorSetPoolPage(ServiceLocator& a_Services,
    const VkDescriptorSetLayout& a_DescriptorSetLayout, const std::map<VkDescriptorType, uint32_t>& a_PoolSizes,
    uint32_t a_Capacity)
    : m_Services(a_Services)
{
    auto device = m_Services.m_LogicalDevice->GetVkDevice();
    auto poolSizes = GeneratePoolSizes(a_PoolSizes, a_Capacity);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.maxSets = a_Capacity;

    vkCreateDescriptorPool(device, &poolInfo, m_Services.m_AllocationCallbacks, &m_VkDescriptorPool);

    m_AvailableDescriptorSets.resize(a_Capacity);
    std::vector<VkDescriptorSetLayout> layouts(a_Capacity, a_DescriptorSetLayout);

    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = m_VkDescriptorPool;
    setAllocInfo.descriptorSetCount = a_Capacity;
    setAllocInfo.pSetLayouts = layouts.data();

    vkAllocateDescriptorSets(device, &setAllocInfo, m_AvailableDescriptorSets.data());
}

krt::DescriptorSetPoolPage::~DescriptorSetPoolPage()
{
    vkDestroyDescriptorPool(m_Services.m_LogicalDevice->GetVkDevice(), m_VkDescriptorPool, m_Services.m_AllocationCallbacks);
}

bool krt::DescriptorSetPoolPage::CanAllocateDescriptorSet()
{
    return !m_AvailableDescriptorSets.empty();
}

std::unique_ptr<krt::DescriptorSetAllocation> krt::DescriptorSetPoolPage::AllocateDescriptorSet()
{
    assert(CanAllocateDescriptorSet());

    auto front = m_AvailableDescriptorSets.front();

    std::swap(m_AvailableDescriptorSets.front(), m_AvailableDescriptorSets.back());
    m_AvailableDescriptorSets.pop_back();

    return std::make_unique<DescriptorSetAllocation>(front, this);    
}

void krt::DescriptorSetPoolPage::FreeDescriptorSet(VkDescriptorSet a_DescriptorSet)
{
    m_AvailableDescriptorSets.push_back(a_DescriptorSet);
}

std::vector<VkDescriptorPoolSize> krt::DescriptorSetPoolPage::GeneratePoolSizes(
    const std::map<VkDescriptorType, uint32_t>& a_PoolSizeMap, uint32_t a_Capacity)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto& poolSizeInfo : a_PoolSizeMap)
    {
        auto& entry = poolSizes.emplace_back();

        entry.descriptorCount = poolSizeInfo.second * a_Capacity;
        entry.type = poolSizeInfo.first;
    }

    return poolSizes;
}
