#pragma once

#include <map>
#include <memory>

namespace krt
{
    class Window;
    class PhysicalDevice;
    class LogicalDevice;
    class DescriptorSetPool;
}

namespace krt
{
    struct ServiceLocator
    {
        VkAllocationCallbacks* m_AllocationCallbacks;

        Window* m_Window;
        PhysicalDevice* m_PhysicalDevice;
        LogicalDevice* m_LogicalDevice;
    };

}