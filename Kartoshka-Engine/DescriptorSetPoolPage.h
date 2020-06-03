#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <memory>
#include <vector>

namespace krt
{
    struct ServiceLocator;
    class DescriptorSetAllocation;
}

namespace krt
{
    class DescriptorSetPoolPage
    {
    public:

        DescriptorSetPoolPage(ServiceLocator& a_Services,const VkDescriptorSetLayout& a_DescriptorSetLayout,
            const std::map<VkDescriptorType, uint32_t>& a_PoolSizes, uint32_t a_Capacity);

        ~DescriptorSetPoolPage();

        bool CanAllocateDescriptorSet();

        std::unique_ptr<DescriptorSetAllocation> AllocateDescriptorSet();

        void FreeDescriptorSet(VkDescriptorSet a_DescriptorSet);

    private:

        std::vector<VkDescriptorPoolSize> GeneratePoolSizes(const std::map<VkDescriptorType, uint32_t>& a_PoolSizeMap, uint32_t a_Capacity);

        ServiceLocator& m_Services;

        VkDescriptorPool m_VkDescriptorPool;

        std::vector<VkDescriptorSet> m_AvailableDescriptorSets;

    };
}

