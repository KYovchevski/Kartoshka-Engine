#include "Window.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "RenderPass.h"
#include "CommandQueue.h"

#include "VkHelpers.h"

krt::Window::Window(ServiceLocator& a_Services, glm::uvec2 a_Size, std::string a_Title)
    : m_Services(a_Services)
    , m_ScreenSize(a_Size)
    , m_WindowTitle(a_Title)
    , m_VkSurface(VK_NULL_HANDLE)
{
    glfwInit();

    // We're not using OpenGL, so no API to initialize
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resizing requires the resizing of the framebuffers, so disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_ScreenSize.x, m_ScreenSize.y, m_WindowTitle.c_str(), nullptr, nullptr);
}

krt::Window::~Window()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void krt::Window::InitializeSwapchain()
{
    auto swapChainDetails = hlp::QuerySwapChainSupportDetails(m_Services.m_PhysicalDevice->GetPhysicalDevice(), m_VkSurface);

    auto surfaceFormat = hlp::ChooseSurfaceFormat(swapChainDetails.m_SurfaceFormats);
    auto presentMode = hlp::ChoosePresentMode(swapChainDetails.m_PresentModes);
    auto extent = hlp::ChooseSwapChainExtent(swapChainDetails.m_SurfaceCapabilities, m_ScreenSize.x, m_ScreenSize.y);

    uint32_t imageCount = swapChainDetails.m_SurfaceCapabilities.minImageCount + 1;
    if (swapChainDetails.m_SurfaceCapabilities.maxImageCount > 0 && imageCount > swapChainDetails.m_SurfaceCapabilities.maxImageCount)
        imageCount = swapChainDetails.m_SurfaceCapabilities.maxImageCount;


    auto queueFamilyIndices = m_Services.m_PhysicalDevice->GetQueueFamilyIndices();
    uint32_t indices[] = { queueFamilyIndices.m_PresentQueueIndex.value(), queueFamilyIndices.m_GraphicsQueueIndex.value() };

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr; 
    createInfo.surface = m_VkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageArrayLayers = 1;
    createInfo.imageExtent = extent;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.presentMode = presentMode;

    createInfo.preTransform = swapChainDetails.m_SurfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (queueFamilyIndices.m_GraphicsQueueIndex.value() == queueFamilyIndices.m_PresentQueueIndex.value())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 1;
        createInfo.pQueueFamilyIndices = &queueFamilyIndices.m_GraphicsQueueIndex.value();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }

    ThrowIfFailed(vkCreateSwapchainKHR(m_Services.m_LogicalDevice->GetVkDevice(), &createInfo, nullptr, &m_VkSwapChain));

    m_SwapChainFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(m_Services.m_LogicalDevice->GetVkDevice(), m_VkSwapChain, &swapChainImageCount, nullptr);
    m_SwapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(m_Services.m_LogicalDevice->GetVkDevice(), m_VkSwapChain, &swapChainImageCount, m_SwapChainImages.data());

    CreateImageViews();
}

void krt::Window::CreateFramebuffers(RenderPass& a_ForwardRenderPass)
{
    m_Framebuffers.reserve(m_SwapChainImages.size());

    for (auto imageView : m_SwapChainImageViews)
    {
        auto& framebuffer =  m_Framebuffers.emplace_back();
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.width = m_ScreenSize.x;
        info.height = m_ScreenSize.y;
        info.pAttachments = &imageView;
        info.attachmentCount = 1;
        info.renderPass = a_ForwardRenderPass.GetVkRenderPass();
        info.layers = 1;
        
        vkCreateFramebuffer(m_Services.m_LogicalDevice->GetVkDevice(), &info, m_Services.m_AllocationCallbacks, &framebuffer);
    }
}

krt::Window::NextFrameInfo krt::Window::GetNextFramebuffer(VkSemaphore a_SemaphoreToSignal, VkFence a_FenceToSignal)
{
    NextFrameInfo info;

    vkAcquireNextImageKHR(m_Services.m_LogicalDevice->GetVkDevice(), m_VkSwapChain, std::numeric_limits<uint64_t>::max(), a_SemaphoreToSignal, a_FenceToSignal, &info.m_FrameIndex);
    info.m_Framebuffer = m_Framebuffers[info.m_FrameIndex];
    return info;
}

void krt::Window::Present(uint32_t a_FrameToPresentIndex, CommandQueue& a_CommandQueue, std::vector<VkSemaphore>& a_SemaphoresToWait)
{
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pSwapchains = &m_VkSwapChain;
    info.pImageIndices = &a_FrameToPresentIndex;
    info.swapchainCount = 1;
    info.pWaitSemaphores = a_SemaphoresToWait.data();
    info.waitSemaphoreCount = static_cast<uint32_t>(a_SemaphoresToWait.size());


    vkQueuePresentKHR(a_CommandQueue.GetVkQueue(), &info);
}

VkFormat krt::Window::GetRenderSurfaceFormat() const
{
    return m_SwapChainFormat;
}

VkSurfaceKHR krt::Window::GetRenderSurface()
{
    if (m_VkSurface == VK_NULL_HANDLE)
        glfwCreateWindowSurface(m_Services.m_LogicalDevice->GetVkInstance(), m_Window, m_Services.m_AllocationCallbacks, &m_VkSurface);
    return m_VkSurface;
}

glm::uvec2 krt::Window::GetScreenSize() const
{
    return m_ScreenSize;
}

std::string krt::Window::GetWindowTitle() const
{
    return m_WindowTitle;
}

VkRect2D krt::Window::GetScreenRenderArea() const
{
    VkRect2D r;
    r.offset.x = 0;
    r.offset.y = 0;
    r.extent.width = static_cast<int32_t>(m_SwapChainExtent.width);
    r.extent.height = static_cast<int32_t>(m_SwapChainExtent.height);
    return r;
}

bool krt::Window::ShouldClose() const
{
    bool shouldClose = glfwWindowShouldClose(m_Window);
    return shouldClose;
}

void krt::Window::PollEvents()
{
    glfwPollEvents();
}

void krt::Window::Resize(glm::uvec2 a_NewSize)
{
    a_NewSize;
    printf("ERROR: Resizing of the window hasn't been implemented yet\n");
}

void krt::Window::SetWindowTitle(std::string a_NewTitle)
{
    m_WindowTitle = a_NewTitle;
    glfwSetWindowTitle(m_Window, m_WindowTitle.c_str());
}

void krt::Window::DestroySwapChain()
{
    const auto dev = m_Services.m_LogicalDevice->GetVkDevice();
    const auto alloc = m_Services.m_AllocationCallbacks;

    for (auto& framebuffer : m_Framebuffers)
        vkDestroyFramebuffer(dev, framebuffer, alloc);

    for (auto& swapChainImageView : m_SwapChainImageViews)
        vkDestroyImageView(dev, swapChainImageView, alloc);

    vkDestroySwapchainKHR(dev, m_VkSwapChain, alloc);
}

void krt::Window::CreateImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.image = m_SwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_SwapChainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;

        ThrowIfFailed(vkCreateImageView(m_Services.m_LogicalDevice->GetVkDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]));
    }
}
