#pragma once

#include "vulkan/vulkan.h"

#include <memory>
#include <vector>

#include "SemaphoreAllocator.h"

namespace krt
{
    struct ServiceLocator;
    class RenderPass;
    class Framebuffer;
}

namespace krt
{
    class VkImGui
    {
    public:
        VkImGui(ServiceLocator& a_Services, RenderPass& a_RenderPass);
        ~VkImGui();

        void Display(uint32_t a_FramebufferIndex, Semaphore a_SignalSemaphore);


    private:
        void CreateRenderPass();

        std::vector<std::unique_ptr<Framebuffer>> m_FrameBuffers;

        ServiceLocator& m_Services;

        VkDescriptorPool m_VkDescriptorPool;

        std::unique_ptr<RenderPass> m_RenderPass;

    };
}

