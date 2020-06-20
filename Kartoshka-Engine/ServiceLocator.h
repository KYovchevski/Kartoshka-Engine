#pragma once


#include "vulkan/vulkan.h"
#include <map>
#include <string>

namespace krt
{
    class Window;
    class PhysicalDevice;
    class LogicalDevice;
    class ModelManager;
    class GraphicsPipeline;
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
        std::map<std::string, GraphicsPipeline*> m_GraphicsPipelines;
    };

}