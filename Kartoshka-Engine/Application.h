#pragma once

#define GLFW_INCLUDE_VULKAN
#include "Window.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <map>

namespace krt
{
    struct ServiceLocator;
    class LogicalDevice;
    class PhysicalDevice;
    class ModelManager;
    class RenderPass;
    class GraphicsPipeline;
    class VertexBuffer;
    class DescriptorSetPool;
    class Texture;
    class Sampler;
    class DepthBuffer;
    class DescriptorSet;
    class IndexBuffer;
    class Material;
    class Scene;
    struct Mesh;
    class PointLight;
    class VkImGui;
    class SemaphoreAllocator;

    class Camera;
    class Transform;
}


namespace krt
{
   

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
        ~Application();

        void Run(const InitializationInfo& a_Info);
    private:
        void Initialize(const InitializationInfo& a_Info);  // Initializes all Vulkan components and window

        void InitializeWindow();

        std::vector<const char*> GetRequiredExtensions() const;

        void                InitializeVulkan(const InitializationInfo& a_Info);
        void                SetupDebugMessenger();

        void                CreateDepthBuffer();

        void                CreateGraphicsPipeline();

        void                CreateRenderPass();
        void                CreateSemaphores();

        void                LoadAssets();

        void                DrawFrame();
        void Cleanup();     // Cleans up after the application is finished running

        void ParseInitializationInfo(const InitializationInfo& a_Info);

        // The callback function used by Vulkan to output validation layer messages to the console
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT a_MessageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT a_MessageType,
            const VkDebugUtilsMessengerCallbackDataEXT* a_CallbackData,
            void* a_UserData);

    private:

        void InitializeImGui();

        void ProcessInput();

        static void FocusCallback(GLFWwindow* a_Window, int a_Focus);

        uint32_t                        m_WindowWidth;
        uint32_t                        m_WindowHeight;
        std::string                     m_WindowTitle;

        VkDebugUtilsMessengerEXT        m_VkDebugMessenger;

        VkSemaphore                     m_ImageAvailableSemaphore;
        VkSemaphore                     m_RenderFinishedSemaphore;

        std::unique_ptr<ServiceLocator> m_ServiceLocator;

        std::unique_ptr<Window>         m_Window;
        std::unique_ptr<DepthBuffer>    m_DepthBuffer;

        std::unique_ptr<PhysicalDevice> m_PhysicalDevice;
        std::unique_ptr<LogicalDevice>  m_LogicalDevice;
        std::unique_ptr<SemaphoreAllocator> m_SemaphoreAllocator;

        std::unique_ptr<ModelManager>   m_ModelManager;

        std::unique_ptr<RenderPass>     m_ForwardRenderPass;
        std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline;
        std::unique_ptr<VkImGui>        m_ImGui;

        std::unique_ptr<Camera>         m_Camera;

        PointLight*                     m_Light;
        PointLight*                     m_Light1;

        std::shared_ptr<Scene>          m_Sponza;

        bool                            m_InFocus;
    };

    
}


