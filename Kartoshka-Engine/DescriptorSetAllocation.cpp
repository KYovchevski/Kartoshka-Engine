#include "DescriptorSetAllocation.h"

#include "DescriptorSetPoolPage.h"

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
