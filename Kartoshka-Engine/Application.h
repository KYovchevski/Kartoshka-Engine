#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <memory>
#include <string>


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
    public:
        void Run(const InitializationInfo& a_Info);
    private:
        void Initialize();  // Initializes all Vulkan components and window

        void InitializeWindow();

        void InitializeVulkan();
        void CreateInstance();

        void Cleanup();     // Cleans up after the application is finished running

        void ParseInitializationInfo(const InitializationInfo& a_Info);

    private:

        uint32_t m_WindowWidth;
        uint32_t m_WindowHeight;
        std::string m_WindowTitle;

        GLFWwindow* m_Window;

        VkInstance m_VkInstance;

    };
}


