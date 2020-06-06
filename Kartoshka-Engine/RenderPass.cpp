#include "RenderPass.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "Framebuffer.h"
#include "VkHelpers.h"

krt::RenderPass::RenderPass(ServiceLocator& a_Services, CreateInfo a_CreateInfo)
    : m_Services(a_Services)
{
    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.attachmentCount = static_cast<uint32_t>(a_CreateInfo.m_Attachments.size());
    createInfo.pAttachments = a_CreateInfo.m_Attachments.data();
    createInfo.subpassCount = static_cast<uint32_t>(a_CreateInfo.m_Subpasses.size());
    createInfo.pSubpasses = a_CreateInfo.m_Subpasses.data();
    createInfo.dependencyCount = static_cast<uint32_t>(a_CreateInfo.m_SubpassDependencies.size());
    createInfo.pDependencies = a_CreateInfo.m_SubpassDependencies.data();

    ThrowIfFailed(vkCreateRenderPass(m_Services.m_LogicalDevice->GetVkDevice(), &createInfo, m_Services.m_AllocationCallbacks, &m_VkRenderPass));
}

krt::RenderPass::~RenderPass()
{
    vkDestroyRenderPass(m_Services.m_LogicalDevice->GetVkDevice(), m_VkRenderPass, m_Services.m_AllocationCallbacks);
}

std::unique_ptr<krt::Framebuffer> krt::RenderPass::CreateFramebuffer()
{
    return std::make_unique<Framebuffer>(m_Services, this);
}

VkRenderPass krt::RenderPass::GetVkRenderPass()
{
    return m_VkRenderPass;
}
