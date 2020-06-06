#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <map>
#include <memory>

namespace krt
{
    struct ServiceLocator;
    class DescriptorSetPoolPage;
    class DescriptorSetAllocation;
}

namespace krt
{
    
    class DescriptorSetPool
    {
    public:
        DescriptorSetPool(ServiceLocator& a_Services, std::vector<VkDescriptorSetLayoutBinding> a_Bindings,
            std::map<VkDescriptorType, uint32_t> a_BindingsMap, uint32_t a_PageCapacity);

        ~DescriptorSetPool();

        DescriptorSetPool(DescriptorSetPool&) = delete;
        DescriptorSetPool operator=(DescriptorSetPool&) = delete;
        DescriptorSetPool(DescriptorSetPool&&) = delete;
        DescriptorSetPool operator=(DescriptorSetPool&&) = delete;

        std::unique_ptr<DescriptorSetAllocation> GetDescriptorSet();

        VkDescriptorSetLayout GetSetLayout() const { return m_VkDescriptorSetLayout; }

        const std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetBindings() const { return m_Bindings; }

    private:

        ServiceLocator& m_Services;

        std::vector<std::unique_ptr<DescriptorSetPoolPage>> m_Pages;
        uint32_t m_PageCapcity;

        std::map<VkDescriptorType, uint32_t> m_BindingsMap;
        std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
        VkDescriptorSetLayout m_VkDescriptorSetLayout;
    };
}
