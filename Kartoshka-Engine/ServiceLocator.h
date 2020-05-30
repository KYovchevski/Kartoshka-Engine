#pragma once

namespace krt
{
    class Window;
    class PhysicalDevice;
    class LogicalDevice;
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