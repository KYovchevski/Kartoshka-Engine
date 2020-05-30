#include "LogicalDevice.h"
#include "ServiceLocator.h"
#include "PhysicalDevice.h"
#include "CommandQueue.h"

#include "VkHelpers.h"
#include "VkConstants.h"

#include "GLFW/glfw3.h"
#include <algorithm>

krt::LogicalDevice::LogicalDevice(ServiceLocator& a_Services, const std::string& a_WindowTitle, const std::string& a_EngineName)
    : m_Services(a_Services)
{
    printf("Creating VkInstance. \n");
    VkApplicationInfo info{};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.apiVersion = VK_VERSION_1_0;
    info.pApplicationName = a_WindowTitle.c_str();
    info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    info.pEngineName = a_EngineName.c_str();
    info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    auto requiredExtensions = GetRequiredExtensions();

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &info;

    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());

    instanceInfo.enabledLayerCount = 0;

#ifdef _DEBUG
    // Enable the necessary validation layers if they are supported
    if (CheckValidationLayerSupport())
    {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(constants::VkValidationLayers.size());
        instanceInfo.ppEnabledLayerNames = constants::VkValidationLayers.data();
        printf("Application running in Debug. Enabling Vulkan validation layers.\n");
    }
    else
        printf("Application running in Debug, but the requested Vulkan validation layers are not supported.\n"
            "Continuing execution without validation layers.\n");
#endif

    ValidateExtensionSupport(requiredExtensions);

    // Create the instance
    ThrowIfFailed(vkCreateInstance(&instanceInfo, nullptr, &m_VkInstance));
}

krt::LogicalDevice::~LogicalDevice()
{
    for (auto& commandQueue : m_CommandQueues)
    {
        commandQueue.second.reset();
    }


    vkDestroyDevice(m_VkLogicalDevice, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
}


VkInstance krt::LogicalDevice::GetVkInstance() const
{
    return m_VkInstance;
}

void krt::LogicalDevice::InitializeDevice()
{
    printf("Creating VkDevice (Logical Device).\n");
    auto queueFamilies = m_Services.m_PhysicalDevice->GetQueueFamilyIndices();

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    // Depending on the system, multiple functionalities can be done by either the same queue family or by multiple queue families
    // Because of this it's important to trim out any duplicates and create queueInfos only for them
    std::set<uint32_t> uniqueQueueFamilies{ queueFamilies.m_PresentQueueIndex.value(), queueFamilies.m_GraphicsQueueIndex.value(),
        queueFamilies.m_TransferQueueIndex.value(), queueFamilies.m_ComputeQueueIndex.value() };
    for (auto& family : uniqueQueueFamilies)
    {
        queueCreateInfo.queueFamilyIndex = family;
        queueInfos.push_back(queueCreateInfo);
    }
    auto requiredExtensions = GetRequiredExtensions();

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};

    assert(ValidateExtensionSupport(requiredExtensions));

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    //deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
    //deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(constants::VkDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = constants::VkDeviceExtensions.data();

#ifdef _DEBUG
    deviceCreateInfo.ppEnabledLayerNames = constants::VkValidationLayers.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(constants::VkValidationLayers.size());
#endif

    ThrowIfFailed(vkCreateDevice(m_Services.m_PhysicalDevice->GetPhysicalDevice(), &deviceCreateInfo, nullptr, &m_VkLogicalDevice));
    m_CommandQueues[EGraphicsQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_GraphicsQueueIndex.value());
    m_CommandQueues[EComputeQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_ComputeQueueIndex.value());
    m_CommandQueues[EPresentQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_PresentQueueIndex.value());
    m_CommandQueues[ETransferQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_TransferQueueIndex.value());
}

VkDevice krt::LogicalDevice::GetVkDevice() const
{
    return m_VkLogicalDevice;
}

krt::CommandQueue& krt::LogicalDevice::GetCommandQueue(const ECommandQueueType a_Type)
{
    return *m_CommandQueues[a_Type];
}

std::vector<const char*> krt::LogicalDevice::GetRequiredExtensions() const
{
    // Get the extensions required for GLFW to function
    uint32_t extCount;
    auto ext = glfwGetRequiredInstanceExtensions(&extCount);

    // If the application is running in debug, add an extra extension for debug utilities
    std::vector extensions(ext, ext + extCount);
#ifdef _DEBUG
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

bool krt::LogicalDevice::CheckValidationLayerSupport() const
{
    // Get the layers supported by Vulkan
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    //auto requiredExtensions = GetRequiredExtensions();

    // Check if all requested validation layers are supported
    for (auto& layer : constants::VkValidationLayers)
    {
        if (std::find_if(layers.begin(), layers.end(), [&](VkLayerProperties& a_Layer)
        {
            return !std::strcmp(a_Layer.layerName, layer);
        }) == layers.end())
            return false;
    }
    return true;
}

bool krt::LogicalDevice::ValidateExtensionSupport(std::vector<const char*> a_Extensions) const
{
    // First query the number of available extensions
    uint32_t numExtensions;
    ThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr));

    // Then make an array for them and store them in it
    std::vector<VkExtensionProperties> extensions(numExtensions);
    ThrowIfFailed(vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensions.data()));

    // Loop through all extensions and check if any of them are not supported
    std::vector<std::string> unsupported;
    for (auto extension : a_Extensions)
    {
        if (std::find_if(extensions.begin(), extensions.end(), [&](VkExtensionProperties& a_Entry)
            {
                return strcmp(a_Entry.extensionName, extension);
            }) == extensions.end())
        {
            unsupported.push_back(extension);
        }
    }

    if (!unsupported.empty())
    {
        printf("The following Vulkan extensions are not supported, but necessary for the execution of the program:\n");
        for (auto& name : unsupported)
        {
            printf("\t%s\n", name.c_str());
        }
        return false;
    }
    else
        printf("All necessary Vulkan extensions are supported.");

    return true;
}

std::vector<VkDeviceQueueCreateInfo> krt::LogicalDevice::GenerateQueueInfos()
{
    auto indices = m_Services.m_PhysicalDevice->GetQueueFamilyIndices().GetUniqueIndices();

    std::vector<VkDeviceQueueCreateInfo> infos;

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    info.pQueuePriorities = &queuePriority;
    info.queueCount = 1;
    for (auto index : indices)
    {
        info.queueFamilyIndex = index;
        infos.push_back(info);
    }
    return infos;
}
