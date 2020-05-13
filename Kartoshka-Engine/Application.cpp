#include "Application.h"
#include "VkHelpers.h"

#include "glm/vec3.hpp"

#include <algorithm>
#include <set>
#include <array>

krt::Application::Application()
    : m_VkPhysicalDevice(VK_NULL_HANDLE)
    , m_SwapChain(VK_NULL_HANDLE)
    , m_VkDebugMessenger(VK_NULL_HANDLE)
    , m_VkGraphicsQueue(VK_NULL_HANDLE)
    , m_VkInstance(VK_NULL_HANDLE)
    , m_VkLogicalDevice(VK_NULL_HANDLE)
    , m_VkPresentQueue(VK_NULL_HANDLE)
    , m_VkSurface(VK_NULL_HANDLE)
    , m_Window(nullptr)
    , m_WindowWidth(0)
    , m_WindowHeight(0)
    , m_WindowTitle("Untitled")
{

}

void krt::Application::Run(const InitializationInfo& a_Info)
{
    ParseInitializationInfo(a_Info);
    Initialize();

    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(m_VkLogicalDevice);

    Cleanup();
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

std::vector<const char*> krt::Application::GetRequiredExtensions() const
{
    // GLFW has some extensions that it needs, so those need to be included
    uint32_t extensionCount;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);

#ifdef _DEBUG
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

void krt::Application::InitializeVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateCommandBuffers();
    CreateSemaphores();
}

void krt::Application::CreateInstance()
{
    printf("Creating VkInstance. \n");
    VkApplicationInfo info{};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.apiVersion = VK_VERSION_1_0;
    info.pApplicationName = m_WindowTitle.c_str();
    info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    info.pEngineName = "Kartoshka-Engine";
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
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(VkValidationLayers.size());
        instanceInfo.ppEnabledLayerNames = VkValidationLayers.data();
        printf("Application running in Debug. Enabling Vulkan validation layers.\n");
    }
    else
        printf("Application running in Debug, but the requested Vulkan validation layers are not supported.\n"
                     "Continuing execution without validation layers.\n");
#endif

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

bool krt::Application::CheckValidationLayerSupport()
{
    // Get the layers supported by Vulkan
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    // Check if all requested validation layers are supported
    for (auto& layer : VkValidationLayers)
    {
        if (std::find_if(layers.begin(), layers.end(), [&](VkLayerProperties& a_Layer)
            {return !std::strcmp(a_Layer.layerName, layer); }) == layers.end())
            return false;
    }
    return true;
}

void krt::Application::SetupDebugMessenger()
{
#ifdef _DEBUG
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
    messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.pfnUserCallback = DebugCallback;
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ThrowIfFailed(vkext::CreateDebugUtilsMessengerEXT(m_VkInstance, messengerInfo, nullptr, m_VkDebugMessenger));
#endif
}

void krt::Application::CreateSurface()
{
    ThrowIfFailed(glfwCreateWindowSurface(m_VkInstance, m_Window, nullptr, &m_VkSurface));
}

void krt::Application::PickPhysicalDevice()
{
    printf("Picking best available physical device. \n");
    // Find the number of physical devices in the system that support vulkan
    uint32_t deviceCount;
    ThrowIfFailed(vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr));

    if (!deviceCount)
    {
        printf("ERROR: Could not find any GPUs with support for Vulkan. Ending execution.\n");
        abort();
    }

    // Queue the actual physical devices
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    ThrowIfFailed(vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, physicalDevices.data()));

    // Find the first suitable physical device
    for (auto& dev : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(dev))
        {
            m_VkPhysicalDevice = dev;
            break;
        }
    }

    if (m_VkPhysicalDevice == VK_NULL_HANDLE)
    {
        printf("ERROR: Could not find suitable GPUs in the system. Ending execution.\n");
        abort();
    }
}

krt::Application::QueueFamilyIndices krt::Application::FindQueueFamilies(VkPhysicalDevice& a_Device)
{
    QueueFamilyIndices indices;
    uint32_t familiesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(a_Device, &familiesCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(a_Device, &familiesCount, families.data());

    for (size_t i = 0; i < families.size(); i++)
    {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !indices.m_GraphicsFamily.has_value())
            indices.m_GraphicsFamily = static_cast<uint32_t>(i);

        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(a_Device, static_cast<uint32_t>(i), m_VkSurface, &presentSupport);
        if (presentSupport)
            indices.m_PresentFamily = static_cast<uint32_t>(i);

        if ((families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && !(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            indices.m_TransferFamily = static_cast<uint32_t>(i);

        if (indices.IsComplete())
            break;
    }

    return indices;
}

bool krt::Application::CheckDeviceExtensionSupport(VkPhysicalDevice& a_Device)
{
    uint32_t extCount;
    vkEnumerateDeviceExtensionProperties(a_Device, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extCount);
    vkEnumerateDeviceExtensionProperties(a_Device, nullptr, &extCount, availableExtensions.data());

    std::set<const char*> requiredExtensions(VkDeviceExtensions.begin(), VkDeviceExtensions.end());

    for (auto& req : requiredExtensions)
    {
        for (auto& ext : availableExtensions)
        {
            if (strcmp(ext.extensionName, req))
            {
                requiredExtensions.erase(req);
                break;
            }
        }

        if (requiredExtensions.empty())
            break;
    }

    return requiredExtensions.empty();
}

bool krt::Application::IsPhysicalDeviceSuitable(VkPhysicalDevice& a_Device)
{
    auto indices = FindQueueFamilies(a_Device);

    bool extensionsSupported = CheckDeviceExtensionSupport(a_Device);

    auto swapChainDetails = hlp::QuerySwapChainSupportDetails(a_Device, m_VkSurface);
    bool swapChainSupported = !swapChainDetails.m_SurfaceFormats.empty() && !swapChainDetails.m_PresentModes.empty();

    return indices.IsComplete() && extensionsSupported && swapChainSupported;
}


void krt::Application::CreateLogicalDevice()
{
    printf("Creating VkDevice (Logical Device).\n");
    auto queueFamilies = FindQueueFamilies(m_VkPhysicalDevice);

    float queuePriority = 1.0f;
    
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    // Depending on the system, multiple functionalities can be done by either the same queue family or by multiple queue families
    // Because of this it's important to trim out any duplicates and create queueInfos only for them
    std::set<uint32_t> uniqueQueueFamilies{ queueFamilies.m_PresentFamily.value(), queueFamilies.m_GraphicsFamily.value(), queueFamilies.m_TransferFamily.value() };
    for (auto& family : uniqueQueueFamilies)
    {
        queueCreateInfo.queueFamilyIndex = family;
        queueInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(VkDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = VkDeviceExtensions.data();

#ifdef _DEBUG
    deviceCreateInfo.ppEnabledLayerNames = VkValidationLayers.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(VkValidationLayers.size());
#endif

    ThrowIfFailed(vkCreateDevice(m_VkPhysicalDevice, &deviceCreateInfo, nullptr, &m_VkLogicalDevice));
    vkGetDeviceQueue(m_VkLogicalDevice, queueFamilies.m_GraphicsFamily.value(), 0, &m_VkGraphicsQueue);
    vkGetDeviceQueue(m_VkLogicalDevice, queueFamilies.m_PresentFamily.value(), 0, &m_VkPresentQueue);
    vkGetDeviceQueue(m_VkLogicalDevice, queueFamilies.m_TransferFamily.value(), 0, &m_TransferQueue);
}

void krt::Application::CreateSwapChain()
{
    auto swapChainDetails = hlp::QuerySwapChainSupportDetails(m_VkPhysicalDevice, m_VkSurface);

    auto surfaceFormat = hlp::ChooseSurfaceFormat(swapChainDetails.m_SurfaceFormats);
    auto presentMode = hlp::ChoosePresentMode(swapChainDetails.m_PresentModes);
    auto extent = hlp::ChooseSwapChainExtent(swapChainDetails.m_SurfaceCapabilities, m_WindowWidth, m_WindowHeight);

    uint32_t imageCount = swapChainDetails.m_SurfaceCapabilities.minImageCount + 1;
    if (swapChainDetails.m_SurfaceCapabilities.maxImageCount > 0 && imageCount > swapChainDetails.m_SurfaceCapabilities.maxImageCount)
        imageCount = swapChainDetails.m_SurfaceCapabilities.maxImageCount;


    auto queueFamilyIndices = FindQueueFamilies(m_VkPhysicalDevice);
    uint32_t indices[] = { queueFamilyIndices.m_PresentFamily.value(), queueFamilyIndices.m_GraphicsFamily.value() };

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

    if (queueFamilyIndices.m_GraphicsFamily.value() == queueFamilyIndices.m_PresentFamily.value())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 1;
        createInfo.pQueueFamilyIndices = &queueFamilyIndices.m_PresentFamily.value();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    }

    ThrowIfFailed(vkCreateSwapchainKHR(m_VkLogicalDevice, &createInfo, nullptr, &m_SwapChain));

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(m_VkLogicalDevice, m_SwapChain, &swapChainImageCount, nullptr);
    m_SwapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(m_VkLogicalDevice, m_SwapChain, &swapChainImageCount, m_SwapChainImages.data());
}

void krt::Application::CreateImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.image = m_SwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_SwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;

        ThrowIfFailed(vkCreateImageView(m_VkLogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]));
    }
}

void krt::Application::CreateGraphicsPipeline()
{
    auto vShaderCode = hlp::LoadFile("../Build/SpirV/BaseVertexShader.spv");
    auto pShaderCode = hlp::LoadFile("../Build/SpirV/BasePixelShader.spv");

    auto vertexModule = hlp::CreateShaderModule(m_VkLogicalDevice, vShaderCode);
    auto pixelModule = hlp::CreateShaderModule(m_VkLogicalDevice, pShaderCode);


    VkPipelineShaderStageCreateInfo vertexStage = {};
    vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStage.module = vertexModule;
    vertexStage.pName = "main";
    vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineShaderStageCreateInfo pixelStage = {};
    pixelStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pixelStage.module = pixelModule;
    pixelStage.pName = "main";
    pixelStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineShaderStageCreateInfo stages[] = { vertexStage, pixelStage };

    auto vertexInputBindings = CreateInputBindings();
    auto vertexInputAttributes = CreateInputAttributes();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();
    vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount =   static_cast<uint32_t>(vertexInputAttributes.size());
    vertexInputInfo.vertexBindingDescriptionCount =     static_cast<uint32_t>(vertexInputBindings.size());

    VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
    assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assemblyInfo.primitiveRestartEnable = VK_FALSE;
    assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport = {};
    viewport.width =  static_cast<float>(m_WindowWidth);
    viewport.height = static_cast<float>(m_WindowHeight);
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissorRect;
    scissorRect.offset = { 0, 0 };
    scissorRect.extent = m_SwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pViewports = &viewport;
    viewportInfo.viewportCount = 1;
    viewportInfo.pScissors = &scissorRect;
    viewportInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterInfo = {};
    rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo.depthClampEnable = VK_FALSE;
    rasterInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterInfo.lineWidth = 1.0f;
    rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterInfo.depthBiasEnable = VK_FALSE;
    rasterInfo.depthBiasClamp = 0.0f;
    rasterInfo.depthBiasConstantFactor = 0.0f;
    rasterInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.minSampleShading = 1.0f;
    multisampleInfo.pSampleMask = nullptr;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = false;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.blendConstants[0] = 0.0f;
    colorBlendInfo.blendConstants[1] = 0.0f;
    colorBlendInfo.blendConstants[2] = 0.0f;
    colorBlendInfo.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[]
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.pDynamicStates = dynamicStates;
    dynamicStateInfo.dynamicStateCount = 2;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    ThrowIfFailed(vkCreatePipelineLayout(m_VkLogicalDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;

    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pInputAssemblyState = &assemblyInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterInfo;
    pipelineInfo.pColorBlendState = &colorBlendInfo;

    pipelineInfo.layout = m_PipelineLayout;

    pipelineInfo.renderPass = m_RenderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    ThrowIfFailed(vkCreateGraphicsPipelines(m_VkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));


    vkDestroyShaderModule(m_VkLogicalDevice, vertexModule, nullptr);
    vkDestroyShaderModule(m_VkLogicalDevice, pixelModule, nullptr);
}

void krt::Application::CreateRenderPass()
{
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = m_SwapChainImageFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachmentReference = {};

    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;


    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;

    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachmentDescription;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDependency;

    ThrowIfFailed(vkCreateRenderPass(m_VkLogicalDevice, &renderPassInfo, nullptr, &m_RenderPass));
}

void krt::Application::CreateFramebuffers()
{
    m_FrameBuffers.reserve(m_SwapChainImageViews.size());

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.height = m_WindowHeight;
    createInfo.width = m_WindowWidth;
    createInfo.layers = 1;
    createInfo.renderPass = m_RenderPass;

    for (auto& imageView: m_SwapChainImageViews)
    {
        createInfo.pAttachments = &imageView;
        auto& fb = m_FrameBuffers.emplace_back();
        ThrowIfFailed(vkCreateFramebuffer(m_VkLogicalDevice, &createInfo, nullptr, &fb));
    }

}

void krt::Application::CreateCommandPool()
{
    VkCommandPoolCreateInfo createInfo = {};

    auto queueIndices = FindQueueFamilies(m_VkPhysicalDevice);

    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueIndices.m_GraphicsFamily.value();

    ThrowIfFailed(vkCreateCommandPool(m_VkLogicalDevice, &createInfo, nullptr, &m_GraphicsCommandPool));

    createInfo.queueFamilyIndex = queueIndices.m_TransferFamily.value();
    ThrowIfFailed(vkCreateCommandPool(m_VkLogicalDevice, &createInfo, nullptr, &m_TransferCommandPool));
}

void krt::Application::CreateCommandBuffers()
{
    m_GraphicsCommandBuffers.resize(m_FrameBuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};

    allocInfo.commandPool = m_GraphicsCommandPool;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_GraphicsCommandBuffers.size());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 3;

    vkAllocateCommandBuffers(m_VkLogicalDevice, &allocInfo, m_GraphicsCommandBuffers.data());

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;

    for (uint32_t i = 0; i < m_GraphicsCommandBuffers.size(); i++)
    {
        vkBeginCommandBuffer(m_GraphicsCommandBuffers[i], &beginInfo);

        VkClearValue clearColor = { 0.4f, 0.5f, 0.9f, 1.0f };
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.framebuffer = m_FrameBuffers[i];
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChainExtent;
        renderPassInfo.pClearValues = &clearColor;
        renderPassInfo.clearValueCount = 1;
        vkCmdBeginRenderPass(m_GraphicsCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_GraphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_GraphicsCommandBuffers[i], 0, 1, &m_VertexBuffer, offsets);
        vkCmdBindVertexBuffers(m_GraphicsCommandBuffers[i], 1, 1, &m_VertexBuffer, offsets);
        vkCmdDraw(m_GraphicsCommandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(m_GraphicsCommandBuffers[i]);
        ThrowIfFailed(vkEndCommandBuffer(m_GraphicsCommandBuffers[i]));
    }

}

void krt::Application::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    ThrowIfFailed(vkCreateSemaphore(m_VkLogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore));
    ThrowIfFailed(vkCreateSemaphore(m_VkLogicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore));
}

void krt::Application::CreateVertexBuffer()
{
    //static const float4 positions[] =
//{
//    float4(0.0f, 0.5f, 0.0f, 1.0f),
//    float4(-0.5f, -0.25f, 0.0f, 1.0f),
//    float4(0.5f, -0.25f, 0.0f, 1.0f)
//};
    std::vector<glm::vec3> positions = 
    {
        {0.0f, 0.5f, 0.0f},
        {-0.5f, -0.25f, 0.0f},
        {0.5f, -0.25f, 0.0f}
    };

    uint32_t stagingBufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t localBufferProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CreateBuffer(positions.size() * sizeof(glm::vec3), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBufferProperties, stagingBuffer, stagingBufferMemory);
    CreateBuffer(positions.size() * sizeof(glm::vec3), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, localBufferProperties, m_VertexBuffer, m_VertexBufferMemory);

    void* data;

    vkMapMemory(m_VkLogicalDevice, stagingBufferMemory, 0, positions.size() * sizeof(glm::vec3), 0, &data);
    memcpy(data, positions.data(), positions.size() * sizeof(glm::vec3));
    vkUnmapMemory(m_VkLogicalDevice, stagingBufferMemory);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = m_TransferCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer transferBuffer;

    vkAllocateCommandBuffers(m_VkLogicalDevice, &allocInfo, &transferBuffer);

    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(transferBuffer, &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.dstOffset = 0;
        copyRegion.srcOffset = 0;
        copyRegion.size = positions.size() * sizeof(glm::vec3);

        vkCmdCopyBuffer(transferBuffer, stagingBuffer, m_VertexBuffer, 1, &copyRegion);

        vkEndCommandBuffer(transferBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &transferBuffer;
        vkQueueSubmit(m_TransferQueue, 1, &submitInfo, VK_NULL_HANDLE);


        vkQueueWaitIdle(m_TransferQueue);
    }

    vkDestroyBuffer(m_VkLogicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_VkLogicalDevice, stagingBufferMemory, nullptr);
    vkFreeCommandBuffers(m_VkLogicalDevice, m_TransferCommandPool, 1, &transferBuffer);
}

void krt::Application::CreateBuffer(VkDeviceSize a_Size, VkBufferUsageFlags a_Usage,
    VkMemoryPropertyFlags a_MemoryProperties, VkBuffer& a_Buffer, VkDeviceMemory& a_BufferMemory)
{
    auto queueIndices = FindQueueFamilies(m_VkPhysicalDevice);


    std::set<uint32_t> concurrentFamiliesSet = { queueIndices.m_TransferFamily.value(), queueIndices.m_GraphicsFamily.value() };
    std::vector<uint32_t> concurrentFamilies(concurrentFamiliesSet.begin(), concurrentFamiliesSet.end());
    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = a_Size;
    createInfo.usage = a_Usage;
    createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = static_cast<uint32_t>(concurrentFamilies.size());
    createInfo.pQueueFamilyIndices = concurrentFamilies.data();


    ThrowIfFailed(vkCreateBuffer(m_VkLogicalDevice, &createInfo, nullptr, &a_Buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_VkLogicalDevice, a_Buffer, &memoryRequirements);


    VkMemoryAllocateInfo memoryInfo = {};
    memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryInfo.allocationSize = memoryRequirements.size;
    memoryInfo.memoryTypeIndex = hlp::FindMemoryType(m_VkPhysicalDevice, memoryRequirements.memoryTypeBits, a_MemoryProperties);
    ThrowIfFailed(vkAllocateMemory(m_VkLogicalDevice, &memoryInfo, nullptr, &a_BufferMemory));

    ThrowIfFailed(vkBindBufferMemory(m_VkLogicalDevice, a_Buffer, a_BufferMemory, 0));
}

std::array<VkVertexInputBindingDescription, 2> krt::Application::CreateInputBindings()
{
    std::array<VkVertexInputBindingDescription, 2> output;

    output[0].binding = 0;
    output[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    output[0].stride = sizeof(glm::vec3);

    output[1].binding = 1;
    output[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    output[1].stride = sizeof(glm::vec3);

    return output;
}

std::array<VkVertexInputAttributeDescription, 2> krt::Application::CreateInputAttributes()
{
    std::array<VkVertexInputAttributeDescription, 2> output;

    output[0].binding = 0;
    output[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    output[0].location = 0;
    output[0].offset = 0;

    output[1].binding = 1;
    output[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    output[1].location = 1;
    output[1].offset = 0;

    return output;
}

void krt::Application::DrawFrame()
{
    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_VkLogicalDevice, m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_GraphicsCommandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    ThrowIfFailed(vkQueueSubmit(m_VkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_SwapChain;
    presentInfo.pImageIndices = &imageIndex;

    ThrowIfFailed(vkQueuePresentKHR(m_VkPresentQueue, &presentInfo));
    vkQueueWaitIdle(m_VkPresentQueue);

}

void krt::Application::Cleanup()
{
#ifdef _DEBUG
    ThrowIfFailed(vkext::DestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDebugMessenger, nullptr));
#endif

    vkDestroyBuffer(m_VkLogicalDevice, m_VertexBuffer, nullptr);
    vkFreeMemory(m_VkLogicalDevice, m_VertexBufferMemory, nullptr);

    vkDestroySemaphore(m_VkLogicalDevice, m_ImageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_VkLogicalDevice, m_RenderFinishedSemaphore, nullptr);

    vkDestroyCommandPool(m_VkLogicalDevice, m_GraphicsCommandPool, nullptr);
    vkDestroyCommandPool(m_VkLogicalDevice, m_TransferCommandPool, nullptr);

    while (!m_FrameBuffers.empty())
    {
        vkDestroyFramebuffer(m_VkLogicalDevice, m_FrameBuffers.back(), nullptr);
        m_FrameBuffers.pop_back();
    }

    while (!m_SwapChainImageViews.empty())
    {
        vkDestroyImageView(m_VkLogicalDevice, m_SwapChainImageViews.back(), nullptr);
        m_SwapChainImageViews.pop_back();
    }
    vkDestroyPipeline(m_VkLogicalDevice, m_Pipeline, nullptr);

    vkDestroyPipelineLayout(m_VkLogicalDevice, m_PipelineLayout, nullptr);
    vkDestroyRenderPass(m_VkLogicalDevice, m_RenderPass, nullptr);

    vkDestroySwapchainKHR(m_VkLogicalDevice, m_SwapChain, nullptr);
    vkDestroyDevice(m_VkLogicalDevice, nullptr);
    vkDestroySurfaceKHR(m_VkInstance, m_VkSurface, nullptr);
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

VkBool32 krt::Application::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*a_MessageSeverity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*a_MessageType*/, const VkDebugUtilsMessengerCallbackDataEXT* a_CallbackData,
    void* /*a_UserData*/)
{
    printf("\nValidation layer: %s\n\n", a_CallbackData->pMessage);

    return VK_FALSE;
}


