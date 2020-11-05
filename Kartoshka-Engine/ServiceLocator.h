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
    class SemaphoreAllocator;
    class RenderPass;
}

namespace krt
{
    enum Pipelines
        : uint16_t
    {
        Forward = 0,
        ShadowMap
    };

    enum RenderPasses
        : uint16_t
    {
        ForwardPass = 0,
        ShadowPass
    };

    struct ServiceLocator
    {
        VkAllocationCallbacks* m_AllocationCallbacks;

        Window* m_Window;
        PhysicalDevice* m_PhysicalDevice;
        LogicalDevice* m_LogicalDevice;
        ModelManager* m_ModelManager;
        SemaphoreAllocator* m_SemaphoreAllocator;

        std::map<Pipelines, GraphicsPipeline*> m_GraphicsPipelines;
        std::map<RenderPasses, RenderPass*> m_RenderPasses;
    };

}