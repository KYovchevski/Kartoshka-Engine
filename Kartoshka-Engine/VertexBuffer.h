#pragma once

#include "vulkan/vulkan.h"

namespace krt
{
    struct ServiceLocator;
    class CommandBuffer;
}

namespace krt
{

    class VertexBuffer
    {
        friend CommandBuffer;
    public:
        VertexBuffer(ServiceLocator& a_Services);
        ~VertexBuffer();

        uint32_t GetElementCount();
        VkBuffer GetVkBuffer() const { return m_VkBuffer; }

    private:

        ServiceLocator& m_Services;

        VkBuffer m_VkBuffer;
        VkDeviceMemory m_VkDeviceMemory;
        uint32_t m_NumElements;
        
    };

}