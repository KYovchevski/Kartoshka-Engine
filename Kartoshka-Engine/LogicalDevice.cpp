#include "LogicalDevice.h"
#include "ServiceLocator.h"
#include "PhysicalDevice.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Buffer.h"

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
    physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
    physicalDeviceFeatures.depthBounds = VK_TRUE;
    physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

    assert(ValidateExtensionSupport(requiredExtensions));

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(constants::VkDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = constants::VkDeviceExtensions.data();

#ifdef _DEBUG
    deviceCreateInfo.ppEnabledLayerNames = constants::VkValidationLayers.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(constants::VkValidationLayers.size());
#endif

    ThrowIfFailed(vkCreateDevice(m_Services.m_PhysicalDevice->GetPhysicalDevice(), &deviceCreateInfo, nullptr, &m_VkLogicalDevice));
    m_CommandQueues[EGraphicsQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_GraphicsQueueIndex.value(), EGraphicsQueue);
    m_CommandQueues[EComputeQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_ComputeQueueIndex.value(), EComputeQueue);
    m_CommandQueues[EPresentQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_PresentQueueIndex.value(), EPresentQueue);
    m_CommandQueues[ETransferQueue] = std::make_unique<CommandQueue>(m_Services, queueFamilies.m_TransferQueueIndex.value(), ETransferQueue);
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

void krt::LogicalDevice::ResizeBuffer(Buffer& a_Buffer, uint64_t a_NewSize, bool a_PreserveContent)
{
    auto oldBuffer = a_Buffer.m_VkBuffer;
    auto oldMemory = a_Buffer.m_VkDeviceMemory;

    auto newElements = CreateBufferElements(a_NewSize, a_Buffer.m_UsageFlags,
        a_Buffer.m_MemoryPropertyFlags, a_Buffer.m_QueuesWithAccess);

    a_Buffer.m_VkBuffer = newElements.first;
    a_Buffer.m_VkDeviceMemory = newElements.second;

    if (a_PreserveContent && a_Buffer.m_BufferSize < a_NewSize)
    {
        auto& transferQueue = GetCommandQueue(ETransferQueue);
        auto& transferBuffer = transferQueue.GetSingleUseCommandBuffer();

        transferBuffer.Begin();
        transferBuffer.BufferCopy(oldBuffer, a_Buffer.m_VkBuffer, a_Buffer.m_BufferSize);
        transferBuffer.End();
        transferBuffer.Submit();
    }

    a_Buffer.m_BufferSize = a_NewSize;

    vkDestroyBuffer(m_VkLogicalDevice, oldBuffer, m_Services.m_AllocationCallbacks);
    vkFreeMemory(m_VkLogicalDevice, oldMemory, m_Services.m_AllocationCallbacks);
}

void krt::LogicalDevice::CopyToDeviceMemory(VkDeviceMemory a_DeviceMemory, const void* a_Data, uint64_t a_DataSize)
{
    void* mem;
    vkMapMemory(m_Services.m_LogicalDevice->GetVkDevice(), a_DeviceMemory, 0, a_DataSize, 0, &mem);
    memcpy(mem, a_Data, a_DataSize);
    vkUnmapMemory(m_Services.m_LogicalDevice->GetVkDevice(), a_DeviceMemory);
}

std::vector<uint32_t> krt::LogicalDevice::GetQueueIndices(const std::set<ECommandQueueType>& a_Queues)
{
    std::set<uint32_t> indicesSet;
    for (auto queue : a_Queues)
    {
        indicesSet.insert(GetCommandQueue(queue).GetFamilyIndex());
    }

    return std::vector<uint32_t>(indicesSet.begin(), indicesSet.end());
}

void krt::LogicalDevice::Flush()
{
    for (auto& pair : m_CommandQueues)
    {
        pair.second->Flush();
    }
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

std::pair<VkBuffer, VkDeviceMemory> krt::LogicalDevice::CreateBufferElements(uint64_t a_Size,
    VkBufferUsageFlags a_Usage, VkMemoryPropertyFlags a_MemoryProperties,
    const std::set<ECommandQueueType>& a_QueuesWithAccess)
{
    VkBufferCreateInfo bufferInfo = {};

    auto queues = GetQueueIndices(a_QueuesWithAccess);

    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;

    bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = a_Size;
    bufferInfo.usage = a_Usage;
    bufferInfo.pQueueFamilyIndices = queues.data();
    bufferInfo.queueFamilyIndexCount = queues.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ThrowIfFailed(vkCreateBuffer(m_VkLogicalDevice, &bufferInfo, m_Services.m_AllocationCallbacks, &vkBuffer));

    auto memoryInfo = m_Services.m_PhysicalDevice->GetMemoryInfoForBuffer(vkBuffer, a_MemoryProperties);

    VkMemoryAllocateInfo alloc = {};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.memoryTypeIndex = memoryInfo.m_MemoryType;
    alloc.allocationSize = memoryInfo.m_Size;

    ThrowIfFailed(vkAllocateMemory(m_VkLogicalDevice, &alloc, m_Services.m_AllocationCallbacks, &vkDeviceMemory));
    ThrowIfFailed(vkBindBufferMemory(m_VkLogicalDevice, vkBuffer, vkDeviceMemory, 0));

    auto buffer = std::make_pair(vkBuffer, vkDeviceMemory);

    return buffer;
}

