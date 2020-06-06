#pragma once

#include "vulkan/vulkan.h"

namespace krt
{
    struct ServiceLocator;
}

namespace krt
{
    class Buffer
    {
    public:
        Buffer(ServiceLocator& a_Services);
        virtual ~Buffer();


        VkBuffer m_VkBuffer;
        VkDeviceMemory m_VkDeviceMemory;
    protected:
        ServiceLocator& m_Services;

    };
}
