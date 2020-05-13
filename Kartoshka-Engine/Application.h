#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>


namespace krt
{
    const std::vector<const char*> VkValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> VkDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    // Struct to define basic variables for the initialization of the application
    struct InitializationInfo
    {
        uint32_t m_Width;       // Width of the screen
        uint32_t m_Height;      // Height of the screen
        std::string m_Title;    // Title of the window

    };

    class Application
    {
        struct QueueFamilyIndices
        {
            bool IsComplete()
            {
                return m_GraphicsFamily.has_value()
                    && m_PresentFamily.has_value()
                    && m_TransferFamily.has_value();
            }
            std::optional<uint32_t> m_GraphicsFamily;
            std::optional<uint32_t> m_PresentFamily;
            std::optional<uint32_t> m_TransferFamily;
        };


    public:
        Application();

        void Run(const InitializationInfo& a_Info);
    private:
        void Initialize();  // Initializes all Vulkan components and window

        void InitializeWindow();

        std::vector<const char*> GetRequiredExtensions() const;

        void                InitializeVulkan();
        void                CreateInstance();
        bool                CheckValidationLayerSupport();
        void                SetupDebugMessenger();

        void                CreateSurface();


        void                PickPhysicalDevice();
        // Ensure that the given physical device is suitable for the purposes of the application
        bool                IsPhysicalDeviceSuitable(VkPhysicalDevice& a_Device);
        // Check what queue families are supported by the given physical device
        QueueFamilyIndices  FindQueueFamilies(VkPhysicalDevice& a_Device);
        bool                CheckDeviceExtensionSupport(VkPhysicalDevice& a_Device);

        void                CreateLogicalDevice();

        void                CreateSwapChain();
        void                CreateImageViews();

        void                CreateGraphicsPipeline();

        void                CreateRenderPass();
        void                CreateFramebuffers();

        void                CreateCommandPool();
        void                CreateCommandBuffers();
        void                CreateSemaphores();

        void                CreateVertexBuffer();

        void                CreateBuffer(VkDeviceSize a_Size, VkBufferUsageFlags a_Usage, VkMemoryPropertyFlags a_MemoryProperties,
                            VkBuffer& a_Buffer, VkDeviceMemory& a_BufferMemory);

        std::array<VkVertexInputBindingDescription, 2> CreateInputBindings();
        std::array<VkVertexInputAttributeDescription, 2> CreateInputAttributes();

        void                DrawFrame();
        void Cleanup();     // Cleans up after the application is finished running

        void ParseInitializationInfo(const InitializationInfo& a_Info);


        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT a_MessageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT a_MessageType,
            const VkDebugUtilsMessengerCallbackDataEXT* a_CallbackData,
            void* a_UserData);

    private:

        uint32_t                        m_WindowWidth;
        uint32_t                        m_WindowHeight;
        std::string                     m_WindowTitle;

        GLFWwindow*                     m_Window;
        VkSurfaceKHR                    m_VkSurface;

        VkDebugUtilsMessengerEXT        m_VkDebugMessenger;
        VkInstance                      m_VkInstance;

        VkPhysicalDevice                m_VkPhysicalDevice;
        VkDevice                        m_VkLogicalDevice;
        VkQueue                         m_VkGraphicsQueue;
        VkQueue                         m_VkPresentQueue;
        VkQueue                         m_TransferQueue;

        VkSwapchainKHR                  m_SwapChain;
        std::vector<VkImage>            m_SwapChainImages;
        std::vector<VkImageView>        m_SwapChainImageViews;
        std::vector<VkFramebuffer>      m_FrameBuffers;
        VkExtent2D                      m_SwapChainExtent;
        VkFormat                        m_SwapChainImageFormat;

        VkRenderPass                    m_RenderPass;
        VkPipelineLayout                m_PipelineLayout;
        VkPipeline                      m_Pipeline;

        VkCommandPool                   m_GraphicsCommandPool;
        std::vector<VkCommandBuffer>    m_GraphicsCommandBuffers;

        VkCommandPool                   m_TransferCommandPool;
        VkCommandPool                   m_TransferCommandBuffers;

        VkSemaphore                     m_ImageAvailableSemaphore;
        VkSemaphore                     m_RenderFinishedSemaphore;
        VkSemaphore                     m_InitialCopiesSemaphore;

        VkBuffer                        m_VertexBuffer;
        VkDeviceMemory                  m_VertexBufferMemory;
    };

    
}


