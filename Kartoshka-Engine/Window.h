#pragma once


#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include "glm/vec2.hpp"

#include <string>
#include <vector>


namespace krt
{
    class CommandQueue;
    struct ServiceLocator;
    class RenderPass;

}

namespace krt
{
    class Window final
    {
        struct NextFrameInfo
        {
            VkFramebuffer m_Framebuffer;
            uint32_t m_FrameIndex;
        };

    public:
        Window(ServiceLocator& a_Services, glm::uvec2 a_Size, std::string a_Title);
        ~Window();

        Window(Window&) = delete; // No copy c-tor
        Window(Window&&) = delete; // No move c-tor
        Window operator=(Window&) = delete; // No copy assignment
        Window operator=(Window&&) = delete; // No move assignment

        void InitializeSwapchain();
        void CreateFramebuffers(RenderPass& a_ForwardRenderPass);

        NextFrameInfo GetNextFramebuffer(VkSemaphore a_SemaphoreToSignal, VkFence a_FenceToSignal = VK_NULL_HANDLE);
        void Present(uint32_t a_FrameToPresentIndex, CommandQueue& a_CommandQueue, std::vector<VkSemaphore>& a_SemaphoresToWait);

        VkFormat GetRenderSurfaceFormat() const;
        VkSurfaceKHR GetRenderSurface();
        glm::uvec2 GetScreenSize() const;
        std::string GetWindowTitle() const;
        VkRect2D GetScreenRenderArea() const;

        bool ShouldClose() const;
        void PollEvents();

        void Resize(glm::uvec2 a_NewSize);
        void SetWindowTitle(std::string a_NewTitle);

        void DestroySwapChain();
    private:

        void CreateImageViews();

        ServiceLocator& m_Services;

        glm::uvec2  m_ScreenSize;
        std::string m_WindowTitle;

        GLFWwindow* m_Window;

        VkSurfaceKHR    m_VkSurface;
        VkSwapchainKHR  m_VkSwapChain;
        VkFormat        m_SwapChainFormat;
        VkExtent2D      m_SwapChainExtent;

        std::vector<VkImage>        m_SwapChainImages;
        std::vector<VkImageView>    m_SwapChainImageViews;
        std::vector<VkFramebuffer>  m_Framebuffers;
    };
}