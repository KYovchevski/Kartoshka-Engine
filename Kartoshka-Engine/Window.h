#pragma once

#include "SemaphoreAllocator.h"

#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include "glm/vec2.hpp"
#include "glm/fwd.hpp"

#include <string>
#include <vector>


namespace krt
{
    class CommandQueue;
    struct ServiceLocator;
    class RenderPass;
    class Framebuffer;
    class DepthBuffer;

}

namespace krt
{
    class Window final
    {
        struct NextFrameInfo
        {
            krt::Framebuffer* m_FrameBuffer;
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

        void CreateFrameBuffers(RenderPass& a_TargetRenderPass);

        NextFrameInfo GetNextFrameInfo(Semaphore a_SemaphoreToSignal, VkFence a_FenceToSignal = VK_NULL_HANDLE);
        void Present(uint32_t a_FrameToPresentIndex, CommandQueue& a_CommandQueue, std::vector<Semaphore>& a_SemaphoresToWait);



        VkFormat GetRenderSurfaceFormat() const;
        VkSurfaceKHR GetRenderSurface();
        glm::uvec2 GetScreenSize() const;
        float GetAspectRatio() const;
        std::string GetWindowTitle() const;
        VkRect2D GetScreenRenderArea() const;
        uint32_t GetMinImageCount() const { return m_MinImageCount; }
        uint32_t GetImageCount() const { return m_ImageCount; }

        std::vector<VkImageView> GetScreenBufferImageViews() { return m_SwapChainImageViews; }

        DepthBuffer& GetDepthBuffer();

        bool ShouldClose() const;
        void PollEvents();

        void Resize(glm::uvec2 a_NewSize);
        void SetWindowTitle(std::string a_NewTitle);
        void SetClearColor(glm::vec4 a_NewClearColor);

        void DestroySwapChain();

        GLFWwindow* GetGLFWwindow() { return m_Window; }
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

        std::unique_ptr<DepthBuffer>    m_DepthBuffer;


        uint32_t m_MinImageCount;
        uint32_t m_ImageCount;

        std::vector<VkImage>                        m_SwapChainImages;
        std::vector<VkImageView>                    m_SwapChainImageViews;
        std::vector<std::unique_ptr<Framebuffer>>   m_Framebuffers;
    };
}