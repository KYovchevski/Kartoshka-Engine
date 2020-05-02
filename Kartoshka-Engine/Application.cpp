#include "Application.h"
#include "VkHelpers.h"



#include <vector>

void krt::Application::Run(const InitializationInfo& a_Info)
{
    ParseInitializationInfo(a_Info);
    Initialize();

    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
    }

}

void krt::Application::Initialize()
{
    InitializeWindow();
    InitializeVulkan();
}

void krt::Application::InitializeWindow()
{
    glfwInit();

    // We're not using OpenGL, so no API to initialize
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resizing requires the resizing of the framebuffers, so disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle.c_str(), nullptr, nullptr);

}

void krt::Application::InitializeVulkan()
{
    CreateInstance();
}

void krt::Application::CreateInstance()
{
    VkApplicationInfo info{};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.apiVersion = VK_VERSION_1_0;
    info.pApplicationName = m_WindowTitle.c_str();
    info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    info.pEngineName = "Kartoshka-Engine";
    info.engineVersion = VK_MAKE_VERSION(1, 0, 0);


    // GLFW has some extensions that it needs, so those need to be included
    uint32_t extensionCount;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &info;

    instanceInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceInfo.enabledExtensionCount = extensionCount;

    instanceInfo.enabledLayerCount = 0;

    // First query the number of available extensions
    uint32_t numExtensions;
    ThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr));

    // Then make an array for them and store them in it
    std::vector<VkExtensionProperties> extensions(numExtensions);
    ThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data()));

    // Output through the console which extensions are available
    printf("Available Vulkan extensions:\n");
    for (auto& extension : extensions)
    {
        printf("\t%s\n", extension.extensionName);
    }

    // Create the instance
    ThrowIfFailed(vkCreateInstance(&instanceInfo, nullptr, &m_VkInstance));
}

void krt::Application::Cleanup()
{
    vkDestroyInstance(m_VkInstance, nullptr);
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void krt::Application::ParseInitializationInfo(const InitializationInfo& a_Info)
{
    m_WindowWidth = a_Info.m_Width;
    m_WindowHeight = a_Info.m_Height;
    m_WindowTitle = a_Info.m_Title;
}

