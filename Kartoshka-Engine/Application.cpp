#include "Application.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Window.h"
#include "ModelManager.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "CommandQueue.h"
#include "VertexBuffer.h"
#include "CommandBuffer.h"
#include "Sampler.h"
#include "DepthBuffer.h"
#include "Framebuffer.h"
#include "DescriptorSet.h"
#include "IndexBuffer.h"
#include "Transform.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "StaticMesh.h"
#include "PointLight.h"

#include "VkHelpers.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include "glm/gtx/vec_swizzle.hpp"

#include "FX-GLTF/gltf.h"

#include <algorithm>
#include <set>
#include <array>

struct Mats
{
    glm::mat4 m_WorldView;
    glm::mat4 m_MVP;
};

krt::Application::Application()
    : m_Window(nullptr)
    , m_WindowWidth(0)
    , m_WindowHeight(0)
    , m_WindowTitle("Untitled")
{

}

krt::Application::~Application()
{
    vkDeviceWaitIdle(m_LogicalDevice->GetVkDevice());
#ifdef _DEBUG
    ThrowIfFailed(vkext::DestroyDebugUtilsMessengerEXT(m_LogicalDevice->GetVkInstance(), m_VkDebugMessenger, nullptr));
#endif

    m_TriangleVertexBuffer.reset();
    vkDestroySemaphore(m_LogicalDevice->GetVkDevice(), m_ImageAvailableSemaphore, m_ServiceLocator->m_AllocationCallbacks);
    vkDestroySemaphore(m_LogicalDevice->GetVkDevice(), m_RenderFinishedSemaphore, m_ServiceLocator->m_AllocationCallbacks);

    m_Window->DestroySwapChain();
    m_GraphicsPipeline.reset();
    m_ForwardRenderPass.reset();

    m_PhysicalDevice.reset();
    m_LogicalDevice.reset();
    m_Window.reset();

}

void krt::Application::Run(const InitializationInfo& a_Info)
{
    ParseInitializationInfo(a_Info);
    Initialize(a_Info);

    while (!m_Window->ShouldClose())
    {
        ProcessInput();
        m_Window->PollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(m_LogicalDevice->GetVkDevice());
}

void krt::Application::Initialize(const InitializationInfo& a_Info)
{
    //InitializeWindow();
    InitializeVulkan(a_Info);
}

void krt::Application::InitializeWindow()
{
    glfwInit();

    // We're not using OpenGL, so no API to initialize
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resizing requires the resizing of the framebuffers, so disable resizing for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle.c_str(), nullptr, nullptr);

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

void krt::Application::InitializeVulkan(const InitializationInfo& a_Info)
{
    m_ServiceLocator = std::make_unique<ServiceLocator>();

    m_Window = std::make_unique<Window>(*m_ServiceLocator, glm::uvec2(a_Info.m_Width, a_Info.m_Height), a_Info.m_Title);
    m_ServiceLocator->m_Window = m_Window.get();

    m_LogicalDevice = std::make_unique<LogicalDevice>(*m_ServiceLocator, m_WindowTitle, "Kartoshka-Engine");
    m_ServiceLocator->m_LogicalDevice = m_LogicalDevice.get();

    auto surface = m_Window->GetRenderSurface();

    m_PhysicalDevice = std::make_unique<PhysicalDevice>(*m_ServiceLocator, surface);
    m_ServiceLocator->m_PhysicalDevice = m_PhysicalDevice.get();

    // The actual initialization of the VkDevice is deferred due to the dependency on the physical device
    m_LogicalDevice->InitializeDevice();
    SetupDebugMessenger();

    CreateDepthBuffer();

    m_Window->InitializeSwapchain();
    CreateRenderPass();
    CreateGraphicsPipeline();

    CreateSemaphores();

    m_ModelManager = std::make_unique<ModelManager>(*m_ServiceLocator);

    LoadAssets();
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
    ThrowIfFailed(vkext::CreateDebugUtilsMessengerEXT(m_ServiceLocator->m_LogicalDevice->GetVkInstance(), messengerInfo, nullptr, m_VkDebugMessenger));
#endif
}

void krt::Application::CreateDepthBuffer()
{
    auto screenSize = m_Window->GetScreenSize();

    auto depthFormat = m_PhysicalDevice->FindSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    std::set<ECommandQueueType> queueTypes = { EGraphicsQueue };
    m_DepthBuffer = std::make_unique<DepthBuffer>(*m_ServiceLocator, screenSize.x, screenSize.y, depthFormat, queueTypes);

}

void krt::Application::CreateGraphicsPipeline()
{
    printf("Creating graphics pipeline.\n");
    glm::uvec2 screenSize = m_Window->GetScreenSize();

    auto colorAttachment = ColorBlendAttachment::CreateDefault();


    GraphicsPipeline::CreateInfo pipelineInfo;
    pipelineInfo.m_FragmentShaderFilepath = "../../../SpirV/Fragment.spv";
    pipelineInfo.m_VertexShaderFilepath = "../../../SpirV/Vertex.spv";
    pipelineInfo.m_DynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    pipelineInfo.m_DynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    pipelineInfo.m_Viewports.push_back({ 0.0f, 0.0f, static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), 0.0f, 1.0f });
    pipelineInfo.m_ScissorRects.push_back({ 0, 0,screenSize.x, screenSize.y });

    pipelineInfo.m_ColorBlendInfo->pAttachments = &colorAttachment;
    pipelineInfo.m_ColorBlendInfo->attachmentCount = 1;

    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec2>(0, 0, VK_FORMAT_R32G32_SFLOAT); // Tex Coords
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec3>(1, 1, VK_FORMAT_R32G32B32_SFLOAT); // Positions
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec4>(2, 2, VK_FORMAT_R32G32B32A32_SFLOAT); // Vertex Colors
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec3>(3, 3, VK_FORMAT_R32G32B32_SFLOAT); // Normals

    pipelineInfo.m_PipelineLayout.AddPushConstantRange<Mats>(VK_SHADER_STAGE_VERTEX_BIT);

    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 0, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    pipelineInfo.m_PipelineLayout.AddLayoutBinding(1, 0, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    pipelineInfo.m_RenderPass = m_ForwardRenderPass.get();
    pipelineInfo.m_SubpassIndex = 0;


    m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(*m_ServiceLocator, pipelineInfo);

    m_ServiceLocator->m_GraphicsPipelines.emplace("Scene", m_GraphicsPipeline.get());

    printf("Graphics pipeline created successfully.\n");
}

void krt::Application::CreateRenderPass()
{
    // Due to me not being able to wrap my head around renderpasses entirely, I decided to keep it like this for now,
    // without making any attempts to automate it to avoid unnecessary headaches until I am confident in understanding it

    // Describes the attachment
    // In this case the attachment is an image from the swap chain which will be presented


    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.format = m_ServiceLocator->m_Window->GetRenderSurfaceFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT; // How many samples to do for geometry edges
    // What to do with the previous data upload loading the attachment at the start of a subpass
    // Since the previous results are irrelevant, it's safe to clear them
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // What to do with the new data at the end of a subpass
    // We want to display it to the user, so we store it in the image
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // Stencil load and store Ops are ignored since the render pass is used for a color format
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = m_DepthBuffer->GetVkFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // The layout the attachment is in before the render pass
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // The layout the attachment will be in after the render pass
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachmentReference = {};
    // The index of the attachment in the attachment descriptions array
    // The array is the pAttachments variable of the VkRenderPassCreateInfo
    attachmentReference.attachment = 0; 
    // The layout we want the attachment in before the subpass is started
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = 1;
    // The order of the color attachments dictates which SV_TARGET they will be in the shader
    subpassDescription.pColorAttachments = &attachmentReference;
    // Currently only graphics subpasses exist, but for future compatibility, these need to be specified explicitly
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.pDepthStencilAttachment = &depthReference; // Attachments for depth stencil buffers
    subpassDescription.pPreserveAttachments = nullptr; // Attachments whose data isn't used in the subpass, but needs to be preserved
    subpassDescription.pInputAttachments = nullptr; // Attachments which are used as input in the shaders
    subpassDescription.pResolveAttachments = nullptr; // Attachments used when doing multisampling for the color attachments

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // The source subpass is from outside the render pass
    subpassDependency.dstSubpass = 0;

    subpassDependency.srcAccessMask = 0; // No access from the source?
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // In the color attachment output stage, after the fragment shader
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Can write to the color attachment
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 

    RenderPass::CreateInfo createInfo;
    createInfo.m_Attachments.push_back(attachmentDescription);
    createInfo.m_Attachments.push_back(depthAttachment);
    createInfo.m_Subpasses.push_back(subpassDescription);
    createInfo.m_SubpassDependencies.push_back(subpassDependency);

    m_ForwardRenderPass = std::make_unique<RenderPass>(*m_ServiceLocator, createInfo);
}

void krt::Application::CreateSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    ThrowIfFailed(vkCreateSemaphore(m_ServiceLocator->m_LogicalDevice->GetVkDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore));
    ThrowIfFailed(vkCreateSemaphore(m_ServiceLocator->m_LogicalDevice->GetVkDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore));
}

void krt::Application::LoadAssets()
{
    printf("Loading assets\n");

    std::vector<glm::vec3> positions = 
    {
        {0.0f, 0.5f, 0.0f},
        {0.5f, -0.0f, 0.0f},
        {-0.5f, -0.0f, 0.0f},
        {0.0f, -0.5f, 0.0f}
    };

    std::vector<uint16_t> indices =
    {
        0, 2, 1,
        1, 2, 3
    };

    std::vector<glm::vec2> tex =
    {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 1.0f}
    };

    auto& transferQueue = m_LogicalDevice->GetCommandQueue(ETransferQueue);
    auto& commandBuffer = transferQueue.GetSingleUseCommandBuffer();


    printf("Creating vertex buffers.\n");
    commandBuffer.Begin();
    m_TriangleVertexBuffer = commandBuffer.CreateVertexBuffer(positions, { EGraphicsQueue });
    m_TexCoords = commandBuffer.CreateVertexBuffer(tex, { EGraphicsQueue });

    printf("Creating index buffer.\n");
    m_Indices = commandBuffer.CreateIndexBuffer(indices, { EGraphicsQueue });

    printf("Loading test texture.\n");
    m_Texture = commandBuffer.CreateTextureFromFile("../../../../Assets/Textures/debugTex.png", {EGraphicsQueue});
    auto samplerInfo = Sampler::CreateInfo::CreateDefault();
    m_Sampler = std::make_unique<Sampler>(*m_ServiceLocator, samplerInfo);

    auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/Sponza/Sponza.gltf");
    //auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/CrowdKing/JL.gltf");
    //auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/Lantern/Lantern.gltf");

    m_Duccc = res->GetMesh();
    m_Sponza = res->GetScene();

    auto light = m_Sponza->AddPointLight();

    light->SetPosition(glm::vec3(0.0f, 15.0f, 0.0f));

    m_Mat = std::make_unique<Material>();
    m_Mat->SetSampler(*m_Sampler);
    m_Mat->SetDiffuseTexture(*m_Texture);

    commandBuffer.Submit();
    printf("Baking descriptor sets.\n");
    glm::vec3 v = { 0.0f, 0.0f, 0.0f };
    std::set<ECommandQueueType> queues = {EGraphicsQueue};
    //m_FrontTriangleSet = std::make_unique<DescriptorSet>(*m_ServiceLocator, *m_GraphicsPipeline, 0u, queues);

    v = { 0.0f, 0.5f, 0.1f };

    //m_BackTriangleSet = std::make_unique<DescriptorSet>(*m_ServiceLocator, *m_GraphicsPipeline, 0u, queues);

    m_Camera = std::make_unique<Camera>();
    m_Camera->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
    m_Camera->SetAspectRatio(m_Window->GetAspectRatio());
    m_Camera->SetFarClipDistance(150.0f);
    m_Sponza->m_ActiveCamera = m_Camera.get();

    m_Transform = std::make_unique<Transform>();
    m_Transform->SetScale(glm::vec3(1.0250f));
    m_Transform->SetScale(glm::vec3(0.0250f));

    transferQueue.Flush();

    printf("All assets loaded.\n");
}

void krt::Application::DrawFrame()
{
    //uint32_t imageIndex;
    //vkAcquireNextImageKHR(m_VkLogicalDevice, m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    auto frameInfo = m_Window->GetNextFrameInfo(m_ImageAvailableSemaphore);

    auto screenSize = m_Window->GetScreenSize();

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(screenSize.y);
    viewport.width = static_cast<float>(screenSize.x);
    viewport.height = -static_cast<float>(screenSize.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0, 0, screenSize.x, screenSize.y};

    auto& commandBuffer = m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetSingleUseCommandBuffer();
    commandBuffer.Begin();

    auto framebuffer = m_ForwardRenderPass->CreateFramebuffer();
    framebuffer->AddImageView(frameInfo.m_ImageView, 0);
    framebuffer->AddTexture(*m_DepthBuffer, 1);
    framebuffer->SetSize(screenSize);

    commandBuffer.BeginRenderPass(*m_ForwardRenderPass, *framebuffer, m_Window->GetScreenRenderArea());
    commandBuffer.SetScissorRect(scissor);
    commandBuffer.SetViewport(viewport);
    commandBuffer.BindPipeline(*m_GraphicsPipeline);

    static float time = 0.0f;
    time += 1.0f / 60.0f;

    glm::mat4 cameraMatrix = m_Camera->GetCameraMatrix();
    glm::mat4 viewMatrix = m_Camera->GetViewMatrix();

    commandBuffer.SetDescriptorSet(m_Sponza->GetLightsDescriptorSet(), 1);
    for (auto& mesh : m_Sponza->m_StaticMeshes)
    {
        auto& m = *mesh;
        glm::mat4 mvp;

        Mats mats;

        mats.m_MVP = cameraMatrix * mesh->m_Transform->GetTransformationMatrix();
        mats.m_WorldView = viewMatrix * mesh->m_Transform->GetTransformationMatrix();

        mvp = cameraMatrix * mesh->m_Transform->GetTransformationMatrix();
        commandBuffer.PushConstant(mats, 0);

        for (auto& primitive : m->m_Primitives)
        {
            commandBuffer.SetVertexBuffer(*primitive.m_TexCoords, 0);
            commandBuffer.SetVertexBuffer(*primitive.m_Positions, 1);
            commandBuffer.SetVertexBuffer(*primitive.m_VertexColors, 2);
            commandBuffer.SetVertexBuffer(*primitive.m_Normals, 3);

            if (primitive.m_Material)
            {
                commandBuffer.SetMaterial(*primitive.m_Material, 0);
            }


            if (primitive.m_IndexBuffer)
            {
                commandBuffer.SetIndexBuffer(*primitive.m_IndexBuffer);
                commandBuffer.DrawIndexed(primitive.m_IndexBuffer->GetElementCount());
            }
            else
            {
                commandBuffer.Draw(primitive.m_Positions->GetElementCount());
            }
        }
    }


    commandBuffer.EndRenderPass();
    commandBuffer.AddSignalSemaphore(m_RenderFinishedSemaphore);
    commandBuffer.AddWaitSemaphore(m_ImageAvailableSemaphore);
    commandBuffer.SetWaitStages(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    commandBuffer.Submit();

    auto& presentQueue = m_ServiceLocator->m_LogicalDevice->GetCommandQueue(EPresentQueue);
    std::vector<VkSemaphore> presentSemaphores;
    presentSemaphores.push_back(m_RenderFinishedSemaphore);
    m_Window->Present(frameInfo.m_FrameIndex, presentQueue, presentSemaphores);
    presentQueue.Flush();
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


void krt::Application::ProcessInput()
{
    auto cameraRotation = glm::mat4_cast(m_Camera->GetRotationQuat());

    auto forward4 = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f) * cameraRotation;
    auto right4 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) *  cameraRotation;

    auto forward = glm::vec3(forward4.x, forward4.y, forward4.z);
    auto right = glm::vec3(right4.x, right4.y, right4.z);

    static const float moveSpeed = 1.0f;

    if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_W))
    {
        m_Camera->Move(forward * moveSpeed / 60.0f);
    }
    else if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_S))
    {
        m_Camera->Move(forward * -moveSpeed / 60.0f);
    }


    if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_A))
    {
        m_Camera->Move(right * -moveSpeed / 60.0f);
    }
    else if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_D))
    {
        m_Camera->Move(right * moveSpeed / 60.0f);
    };


    if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_Q))
    {
        m_Camera->Move(glm::vec3(0.0f, -1.0f * moveSpeed / 60.0f, 0.0f));
    }
    else if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_E))
    {
        m_Camera->Move(glm::vec3(0.0f, 1.0f * moveSpeed / 60.0f, 0.0f));
    };

    if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_LEFT))
    {
        m_Camera->Rotate(glm::vec3(0.0f, 30.0f * 1.0f / 60.0f, 0.0f));
    }
    else if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_RIGHT))
    {
        m_Camera->Rotate(glm::vec3(0.0f, -30.0f * 1.0f / 60.0f, 0.0f));
    };
}
