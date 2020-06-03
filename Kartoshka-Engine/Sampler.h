#pragma once

#include "vulkan/vulkan.h"

namespace krt
{
    struct ServiceLocator;
    class CommandBuffer;
}

namespace krt
{
    class Sampler
    {
        friend CommandBuffer;
    public:

        class CreateInfo
        {
        public:
            CreateInfo();

            static CreateInfo CreateDefault();

            VkSamplerCreateInfo* operator->();
            VkSamplerCreateInfo* operator&();

        private:
            VkSamplerCreateInfo m_VkSamplerCreateInfo;
        };

        Sampler(ServiceLocator& a_Services, CreateInfo a_CreateInfo);
        ~Sampler();

    private:
        ServiceLocator& m_Services;

        VkSampler m_VkSampler;

    };
}
