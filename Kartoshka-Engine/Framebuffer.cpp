#include "Framebuffer.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "RenderPass.h"
#include "Texture.h"

#include "glm/vec4.hpp"

krt::Framebuffer::Framebuffer(ServiceLocator& a_Services, RenderPass* a_CompatibleRenderPass)
    : m_Services(a_Services)
    , m_CompatibleRenderPass(a_CompatibleRenderPass)
{
    
}

krt::Framebuffer::~Framebuffer()
{
    DestroyVkFramebuffer();
}

void krt::Framebuffer::AddTexture(Texture& a_Texture, uint32_t a_Slot)
{
    m_Attachments[a_Slot].m_ImageView = a_Texture.GetVkImageView();
    m_Dirty = true;
}

void krt::Framebuffer::AddImageView(VkImageView a_ImageView, uint32_t a_Slot)
{
    m_Attachments[a_Slot].m_ImageView = a_ImageView;
    m_Dirty = true;
}

void krt::Framebuffer::SetClearValues(glm::vec4 a_ClearColor, uint32_t a_Slot)
{
    m_Attachments[a_Slot].m_ClearValue.color.float32[0] = a_ClearColor.x;
    m_Attachments[a_Slot].m_ClearValue.color.float32[1] = a_ClearColor.y;
    m_Attachments[a_Slot].m_ClearValue.color.float32[2] = a_ClearColor.z;
    m_Attachments[a_Slot].m_ClearValue.color.float32[3] = a_ClearColor.w;
}

void krt::Framebuffer::SetClearValues(float a_Depth, uint32_t a_Stencil, uint32_t a_Slot)
{
    m_Attachments[a_Slot].m_ClearValue.depthStencil.depth = a_Depth;
    m_Attachments[a_Slot].m_ClearValue.depthStencil.stencil = a_Stencil;
}

VkFramebuffer krt::Framebuffer::GetVkFrameBuffer()
{
    if (m_Dirty)
    {
        DestroyVkFramebuffer();
        MakeVkFrameBuffer();
        m_Dirty = false;
    }

    return m_VkFramebuffer;
}

void krt::Framebuffer::SetSize(glm::uvec2 a_NewSize)
{
    m_Size = a_NewSize;
    m_Dirty = true;
}

std::vector<VkClearValue> krt::Framebuffer::GetClearValues() const
{
    std::vector<VkClearValue> clearValues;
    clearValues.reserve(m_Attachments.size());
    for (auto& pair : m_Attachments)
    {
        clearValues.push_back(pair.second.m_ClearValue);
    }
    return clearValues;
}

void krt::Framebuffer::DestroyVkFramebuffer()
{
    if (m_VkFramebuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(m_Services.m_LogicalDevice->GetVkDevice(), m_VkFramebuffer, m_Services.m_AllocationCallbacks);
    }
}

void krt::Framebuffer::MakeVkFrameBuffer()
{
    std::vector<VkImageView> attachments;
    attachments.reserve(m_Attachments.size());
    for (auto& attachmentImageView : m_Attachments)
    {
        attachments.push_back(attachmentImageView.second.m_ImageView);
    }

    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.width = m_Size.x;
    info.height = m_Size.y;
    info.layers = 1;
    info.renderPass = m_CompatibleRenderPass->GetVkRenderPass();

    vkCreateFramebuffer(m_Services.m_LogicalDevice->GetVkDevice(), &info, m_Services.m_AllocationCallbacks, &m_VkFramebuffer);
}
