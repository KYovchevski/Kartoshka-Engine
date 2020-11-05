#pragma once

#include "vulkan/vulkan.h"

#include "glm/vec2.hpp"
#include "glm/fwd.hpp"

#include <map>
#include <vector>

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

        void SetClearValues(glm::vec4 a_ClearColor, uint32_t a_Slot);
        void SetClearValues(float a_Depth, uint32_t a_Stencil, uint32_t a_Slot);

        VkFramebuffer GetVkFrameBuffer();

        void SetSize(glm::uvec2 a_NewSize);

        std::vector<VkClearValue> GetClearValues() const;

    private:

        void DestroyVkFramebuffer();
        void MakeVkFrameBuffer();

        struct Attachment
        {
            Attachment()
                : m_ImageView(VK_NULL_HANDLE)
            {
                m_ClearValue.depthStencil.depth = 1.0f;
                m_ClearValue.depthStencil.stencil = 0;
            }
            VkImageView m_ImageView;
            VkClearValue m_ClearValue;
        };

        std::map<uint32_t, Attachment> m_Attachments;

        ServiceLocator& m_Services;

        VkFramebuffer m_VkFramebuffer;
        bool m_Dirty;


        RenderPass* m_CompatibleRenderPass;
        glm::uvec2 m_Size;
    };
}
