#pragma once


#include <memory>
#include <vector>

#include "vulkan/vulkan.h"

namespace krt
{
    struct ServiceLocator;
    class Application;
    class Framebuffer;
}

namespace krt
{

    // Wrapper struct for VkRenderPass
    // Currently has little in terms of functionality
    // TODO: Automate this more so that the user doesn't have to specify everything
    class RenderPass
    {
        friend Application;
    public:
        struct CreateInfo
        {
            std::vector<VkAttachmentDescription> m_Attachments;
            std::vector<VkSubpassDescription> m_Subpasses;
            std::vector<VkSubpassDependency> m_SubpassDependencies;            
        };

        RenderPass(ServiceLocator& a_Services, CreateInfo a_CreateInfo);
        ~RenderPass();

        std::unique_ptr<Framebuffer> CreateFramebuffer();


        VkRenderPass GetVkRenderPass(); 

    private:
        ServiceLocator& m_Services;
        VkRenderPass m_VkRenderPass;

    };

}