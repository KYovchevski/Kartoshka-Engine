#include "Framebuffer.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "RenderPass.h"
#include "Texture.h"

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
    m_AttachmentImageViews[a_Slot] = a_Texture.GetVkImageView();
    m_Dirty = true;
}

void krt::Framebuffer::AddImageView(VkImageView a_ImageView, uint32_t a_Slot)
{
    m_AttachmentImageViews[a_Slot] = a_ImageView;
    m_Dirty = true;
}

VkFramebuffer krt::Framebuffer::GetVkFrameBuffer()
{
    if (m_Dirty)
    {
        DestroyVkFramebuffer();
        MakeVkFrameBuffer();
    }

    return m_VkFramebuffer;
}

void krt::Framebuffer::SetSize(glm::uvec2 a_NewSize)
{
    m_Size = a_NewSize;
    m_Dirty = true;
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
    attachments.reserve(m_AttachmentImageViews.size());
    for (auto& attachmentImageView : m_AttachmentImageViews)
    {
        attachments.push_back(attachmentImageView.second);
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
