#pragma once

#include "vulkan/vulkan.h"

namespace krt
{
    class DescriptorSetPoolPage;
}

namespace krt
{
    class DescriptorSetAllocation
    {
    public:
        DescriptorSetAllocation(VkDescriptorSet a_DescriptorSet, DescriptorSetPoolPage* a_OriginPage);
        ~DescriptorSetAllocation();
        DescriptorSetAllocation(DescriptorSetAllocation&&);
        DescriptorSetAllocation operator=(DescriptorSetAllocation&&);

        DescriptorSetAllocation(DescriptorSetAllocation&) = delete;
        DescriptorSetAllocation& operator=(DescriptorSetAllocation&) = delete;

        VkDescriptorSet operator*() const;

    private:
        VkDescriptorSet m_VkDescriptorSet;
        DescriptorSetPoolPage* m_OriginPage;
    };
}
