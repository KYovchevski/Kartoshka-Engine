#pragma once

#include "vulkan/vulkan.h"

#include <memory>
#include <vector>

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

        void Display(VkImageView a_ScreenImageView, std::vector<VkSemaphore> a_SignalSemaphores);


    private:
        void CreateRenderPass();

        std::unique_ptr<Framebuffer> m_FrameBuffer;

        ServiceLocator& m_Services;

        VkDescriptorPool m_VkDescriptorPool;

        std::unique_ptr<RenderPass> m_RenderPass;

    };
}

