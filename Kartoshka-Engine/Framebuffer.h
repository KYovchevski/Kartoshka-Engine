#pragma once

#include "vulkan/vulkan.h"

#include "glm/vec2.hpp"

#include <map>

namespace krt
{
    struct ServiceLocator;
    class RenderPass;
    class Texture;
}

namespace krt
{
    class Framebuffer
    {
    public:
        Framebuffer(ServiceLocator& a_Services, RenderPass* a_CompatibleRenderPass);

        ~Framebuffer();

        void AddTexture(Texture& a_Texture, uint32_t a_Slot);
        void AddImageView(VkImageView a_ImageView, uint32_t a_Slot);

        VkFramebuffer GetVkFrameBuffer();

        void SetSize(glm::uvec2 a_NewSize);

    private:

        void DestroyVkFramebuffer();
        void MakeVkFrameBuffer();

        std::map<uint32_t, VkImageView> m_AttachmentImageViews;

        ServiceLocator& m_Services;

        VkFramebuffer m_VkFramebuffer;
        bool m_Dirty;


        RenderPass* m_CompatibleRenderPass;
        glm::uvec2 m_Size;
    };
}
