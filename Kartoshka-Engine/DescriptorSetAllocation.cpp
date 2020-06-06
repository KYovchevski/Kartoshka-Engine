#include "DescriptorSetAllocation.h"

#include "DescriptorSetPoolPage.h"
#include "DescriptorSetPool.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"

#include <cassert>

krt::DescriptorSetAllocation::DescriptorSetAllocation(VkDescriptorSet a_DescriptorSet, DescriptorSetPoolPage* a_OriginPage)
    : m_VkDescriptorSet(a_DescriptorSet)
    , m_OriginPage(a_OriginPage)
{
}

krt::DescriptorSetAllocation::~DescriptorSetAllocation()
{
    if (m_VkDescriptorSet != VK_NULL_HANDLE)
    {
        m_OriginPage->FreeDescriptorSet(m_VkDescriptorSet);
    }
}

krt::DescriptorSetAllocation::DescriptorSetAllocation(DescriptorSetAllocation&& a_Other)
{
    m_VkDescriptorSet = a_Other.m_VkDescriptorSet;
    m_OriginPage = a_Other.m_OriginPage;

    a_Other.m_VkDescriptorSet = VK_NULL_HANDLE;
}

krt::DescriptorSetAllocation krt::DescriptorSetAllocation::operator=(DescriptorSetAllocation&& a_Other)
{
    DescriptorSetAllocation newAlloc(a_Other.m_VkDescriptorSet, a_Other.m_OriginPage);
    a_Other.m_VkDescriptorSet = VK_NULL_HANDLE;

    return newAlloc;
}

VkDescriptorSet krt::DescriptorSetAllocation::operator*() const
{
    return m_VkDescriptorSet;
}

std::vector<VkCopyDescriptorSet> krt::DescriptorSetAllocation::GetCopyInfos(VkDescriptorSet a_ToCopyFrom)
{
    auto& bindings = m_OriginPage->GetOriginPool().GetDescriptorSetBindings();

    std::vector<VkCopyDescriptorSet> copies;
    copies.reserve(bindings.size());

    for (auto& binding : bindings)
    {
        VkCopyDescriptorSet& copy = copies.emplace_back();
        copy = {};
        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

        copy.srcSet = a_ToCopyFrom;
        copy.srcBinding = binding.binding;
        copy.srcArrayElement = 0;

        copy.dstSet = m_VkDescriptorSet;
        copy.dstBinding = binding.binding;
        copy.dstArrayElement = 0;

        copy.descriptorCount = binding.descriptorCount;
    }

    //auto& services = m_OriginPage->GetServices();
    //
    //vkUpdateDescriptorSets(services.m_LogicalDevice->GetVkDevice(),
    //    0, nullptr, static_cast<uint32_t>(copies.size()), copies.data());
    return copies;
}
