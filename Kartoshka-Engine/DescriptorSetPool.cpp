#include "DescriptorSetPool.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"

#include "DescriptorSetPoolPage.h"
#include "DescriptorSetAllocation.h"

#include "VkHelpers.h"


krt::DescriptorSetPool::DescriptorSetPool(ServiceLocator& a_Services, std::vector<VkDescriptorSetLayoutBinding> a_Bindings,
    std::map<VkDescriptorType, uint32_t> a_BindingsMap, uint32_t a_PageCapacity)
    : m_Services(a_Services)
    , m_BindingsMap(a_BindingsMap)
    , m_PageCapcity(a_PageCapacity)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = a_Bindings.data();
    layoutInfo.bindingCount = static_cast<uint32_t>(a_Bindings.size());

    vkCreateDescriptorSetLayout(m_Services.m_LogicalDevice->GetVkDevice(), &layoutInfo, m_Services.m_AllocationCallbacks, &m_VkDescriptorSetLayout);
}

krt::DescriptorSetPool::~DescriptorSetPool()
{
}

std::unique_ptr<krt::DescriptorSetAllocation> krt::DescriptorSetPool::GetDescriptorSet()
{
    DescriptorSetPoolPage* pageToAllocateOn = nullptr;

    for (auto& page : m_Pages)
    {
        if (page->CanAllocateDescriptorSet())
        {
            pageToAllocateOn = page.get();
            break;
        }
    }

    if (pageToAllocateOn)
        return pageToAllocateOn->AllocateDescriptorSet();

    auto& newPage = m_Pages.emplace_back(std::make_unique<DescriptorSetPoolPage>(m_Services, m_VkDescriptorSetLayout, m_BindingsMap, m_PageCapcity));

    return newPage->AllocateDescriptorSet();
}
