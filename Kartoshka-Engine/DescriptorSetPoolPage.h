#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <memory>
#include <vector>

namespace krt
{
    struct ServiceLocator;
    class DescriptorSetAllocation;
    class DescriptorSetPool;
}

namespace krt
{
    class DescriptorSetPoolPage
    {
    public:

        DescriptorSetPoolPage(ServiceLocator& a_Services,const VkDescriptorSetLayout& a_DescriptorSetLayout,
            const std::map<VkDescriptorType, uint32_t>& a_PoolSizes, uint32_t a_Capacity, DescriptorSetPool& a_OriginPool);

        ~DescriptorSetPoolPage();

        bool CanAllocateDescriptorSet();

        std::unique_ptr<DescriptorSetAllocation> AllocateDescriptorSet();

        void FreeDescriptorSet(VkDescriptorSet a_DescriptorSet);

        const DescriptorSetPool& GetOriginPool() const { return m_OriginPool; }

        ServiceLocator& GetServices() const { return m_Services; }

    private:

        std::vector<VkDescriptorPoolSize> GeneratePoolSizes(const std::map<VkDescriptorType, uint32_t>& a_PoolSizeMap, uint32_t a_Capacity);

        ServiceLocator& m_Services;

        DescriptorSetPool& m_OriginPool;

        VkDescriptorPool m_VkDescriptorPool;

        std::vector<VkDescriptorSet> m_AvailableDescriptorSets;

    };
}

