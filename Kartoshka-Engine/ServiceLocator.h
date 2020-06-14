#pragma once

#include "vulkan/vulkan.h"

namespace krt
{
    class Window;
    class PhysicalDevice;
    class LogicalDevice;
    class ModelManager;
}

namespace krt
{
    struct ServiceLocator
    {
        VkAllocationCallbacks* m_AllocationCallbacks;

        Window* m_Window;
        PhysicalDevice* m_PhysicalDevice;
        LogicalDevice* m_LogicalDevice;
        ModelManager* m_ModelManager;
    };

}