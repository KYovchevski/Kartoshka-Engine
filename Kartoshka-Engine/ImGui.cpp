#include "ImGui.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "CommandQueue.h"
#include "Window.h"
#include "RenderPass.h"
#include "CommandBuffer.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

krt::VkImGui::VkImGui(ServiceLocator& a_Services, RenderPass&)
    : m_Services(a_Services)
{

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * 9;
    pool_info.poolSizeCount = 9;
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(m_Services.m_LogicalDevice->GetVkDevice(), &pool_info, m_Services.m_AllocationCallbacks, &m_VkDescriptorPool);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForVulkan(m_Services.m_Window->GetGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo info = {};
    info.DescriptorPool = m_VkDescriptorPool;
    info.Instance = m_Services.m_LogicalDevice->GetVkInstance();
    info.PhysicalDevice = m_Services.m_PhysicalDevice->GetPhysicalDevice();
    info.Device = m_Services.m_LogicalDevice->GetVkDevice();
    info.Allocator = m_Services.m_AllocationCallbacks;
    info.MinImageCount = m_Services.m_Window->GetMinImageCount();
    info.ImageCount = m_Services.m_Window->GetImageCount();
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.PipelineCache = VK_NULL_HANDLE;
    info.Queue = m_Services.m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetVkQueue();
    info.QueueFamily = m_Services.m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetFamilyIndex();

    CreateRenderPass();

    ImGui_ImplVulkan_Init(&info, m_RenderPass->GetVkRenderPass());

    auto& commandBuffer = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue).GetSingleUseCommandBuffer();
    commandBuffer.Begin();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer.GetVkCommandBuffer());
    commandBuffer.Submit();


    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

}



krt::VkImGui::~VkImGui()
{
    vkDestroyDescriptorPool(m_Services.m_LogicalDevice->GetVkDevice(), m_VkDescriptorPool, m_Services.m_AllocationCallbacks);

    ImGui_ImplVulkan_Shutdown();
}

void krt::VkImGui::Display(VkImageView a_ScreenImageView, std::vector<Semaphore> a_SignalSemaphores)
{
    ImGui::Render();

    auto drawData = ImGui::GetDrawData();
    auto& commandBuffer = m_Services.m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetSingleUseCommandBuffer();
    commandBuffer.Begin();

    auto screenSize = m_Services.m_Window->GetScreenSize();

    m_FrameBuffer = m_RenderPass->CreateFramebuffer();
    m_FrameBuffer->AddImageView(a_ScreenImageView, 0);
    m_FrameBuffer->SetSize(screenSize);

    commandBuffer.BeginRenderPass(*m_RenderPass, *m_FrameBuffer, m_Services.m_Window->GetScreenRenderArea());

    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer.GetVkCommandBuffer());

    commandBuffer.EndRenderPass();

    for (auto& signalSemaphore : a_SignalSemaphores)
    {
        commandBuffer.AddSignalSemaphore(signalSemaphore);
    }

    commandBuffer.Submit();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void krt::VkImGui::CreateRenderPass()
{
    VkAttachmentDescription attachment = {};
    attachment.format = m_Services.m_Window->GetRenderSurfaceFormat();
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    RenderPass::CreateInfo info;
    info.m_Subpasses.push_back(subpass);
    info.m_Attachments.push_back(attachment);
    info.m_SubpassDependencies.push_back(dependency);

    m_RenderPass = std::make_unique<RenderPass>(m_Services, info);
}
