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
#include "ImGui.h"
#include "SemaphoreAllocator.h"
#include "CubeShadowMap.h"

#include "VkHelpers.h"

#define GLM_
#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include "glm/gtx/vec_swizzle.hpp"

#include "ImGui/imgui.h"

#include "FX-GLTF/gltf.h"

#include <algorithm>
#include <set>
#include <array>

// Global pointer to the application so that the focus function can find it
krt::Application* g_Application;

struct Mats
{
    glm::mat4 m_World;
    glm::mat4 m_MVP;
};

krt::Application::Application()
    : m_Window(nullptr)
    , m_WindowWidth(0)
    , m_WindowHeight(0)
    , m_WindowTitle("Untitled")
    , m_InFocus(true)
{
    g_Application = this;
}

krt::Application::~Application()
{
    vkDeviceWaitIdle(m_LogicalDevice->GetVkDevice());
#ifdef _DEBUG
    ThrowIfFailed(vkext::DestroyDebugUtilsMessengerEXT(m_LogicalDevice->GetVkInstance(), m_VkDebugMessenger, nullptr));
#endif

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

    Semaphore lastFrameSemaphore = nullptr;

    while (!m_Window->ShouldClose())
    {
        m_Window->PollEvents();
        if (!m_InFocus)
            continue;
        ProcessInput();
        lastFrameSemaphore = DrawFrame(lastFrameSemaphore);
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

    m_Window->InitializeSwapchain();
    CreateRenderPass();
    CreateGraphicsPipeline();

    m_SemaphoreAllocator = std::make_unique<SemaphoreAllocator>(*m_ServiceLocator);
    m_ServiceLocator->m_SemaphoreAllocator = m_SemaphoreAllocator.get();

    m_ModelManager = std::make_unique<ModelManager>(*m_ServiceLocator);

    m_Window->CreateFrameBuffers(*m_ForwardRenderPass);

    m_Window->SetClearColor(glm::vec4(0.4f, 0.5f, 0.9f, 1.0f));

    LoadAssets();

    InitializeImGui();

    glfwSetWindowFocusCallback(m_Window->GetGLFWwindow(), FocusCallback);
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


void krt::Application::CreateGraphicsPipeline()
{
    printf("Creating graphics pipeline.\n");
    glm::uvec2 screenSize = m_Window->GetScreenSize();

    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;

#pragma region GeometryPipeline
    GraphicsPipeline::CreateInfo pipelineInfo;
    pipelineInfo.m_FragmentShaderFilepath = "../../../SpirV/Fragment.spv";
    pipelineInfo.m_VertexShaderFilepath = "../../../SpirV/Vertex.spv";
    pipelineInfo.m_DynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    pipelineInfo.m_DynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    pipelineInfo.m_Viewports.push_back({ 0.0f, 0.0f, static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), 0.0f, 1.0f });
    pipelineInfo.m_ScissorRects.push_back({ 0, 0,screenSize.x, screenSize.y });

    auto colorAttachment = ColorBlendAttachment::CreateDefault();
    pipelineInfo.m_ColorBlendInfo->pAttachments = &colorAttachment;
    pipelineInfo.m_ColorBlendInfo->attachmentCount = 1;

    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec3>(0, 0, VK_FORMAT_R32G32B32_SFLOAT); // Positions
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec2>(1, 1, VK_FORMAT_R32G32_SFLOAT); // Tex Coords
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec4>(2, 2, VK_FORMAT_R32G32B32A32_SFLOAT); // Vertex Colors
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec3>(3, 3, VK_FORMAT_R32G32B32_SFLOAT); // Normals
    pipelineInfo.m_VertexInput.AddPerVertexAttribute<glm::vec4>(4, 4, VK_FORMAT_R32G32B32A32_SFLOAT); // Tangents

    pipelineInfo.m_PipelineLayout.AddPushConstantRange<Mats>(VK_SHADER_STAGE_VERTEX_BIT);

    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 0, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(0, 3, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

    pipelineInfo.m_PipelineLayout.AddLayoutBinding(1, 0, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(1, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    pipelineInfo.m_PipelineLayout.AddLayoutBinding(1, 2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER);

    pipelineInfo.m_RenderPass = m_ForwardRenderPass.get();
    pipelineInfo.m_SubpassIndex = 0;

    pipelineInfo.m_RasterizationStateInfo->cullMode = VK_CULL_MODE_NONE;

    m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(*m_ServiceLocator, pipelineInfo);

    m_ServiceLocator->m_GraphicsPipelines.emplace(Forward, m_GraphicsPipeline.get());
    
#pragma endregion 
#pragma region ShadowMapPipeline

    GraphicsPipeline::CreateInfo shadowMapPipeline;

    shadowMapPipeline.m_VertexShaderFilepath = "../../../SpirV/ShadowVertex.spv";
    shadowMapPipeline.m_DynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    shadowMapPipeline.m_DynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    shadowMapPipeline.m_Viewports.push_back(VkViewport{ 0, 0, 512, 512, 0.0f, 1.0f });
    shadowMapPipeline.m_ScissorRects.push_back({ 0, 0, 512, 512 });

    //shadowMapPipeline.m_VertexInput.AddPerVertexAttribute<glm::vec3>(0, 0, VK_FORMAT_R32G32B32_SFLOAT); // Position
    shadowMapPipeline.m_VertexInput.AddPerVertexAttribute<glm::vec3>(0, 0, VK_FORMAT_R32G32B32_SFLOAT); // Positions

    shadowMapPipeline.m_PipelineLayout.AddPushConstantRange<glm::mat4>(VK_SHADER_STAGE_VERTEX_BIT);

    shadowMapPipeline.m_RenderPass = m_ShadowRenderPass.get();
    shadowMapPipeline.m_SubpassIndex = 0;
    shadowMapPipeline.m_RasterizationStateInfo->cullMode = cullMode ^ 3; // No, i actually don't care anymore

    m_ShadowPipeline = std::make_unique<GraphicsPipeline>(*m_ServiceLocator, shadowMapPipeline);
    m_ServiceLocator->m_GraphicsPipelines.emplace(ShadowMap, m_ShadowPipeline.get());

#pragma endregion

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
    depthAttachment.format = m_Window->GetDepthBuffer().GetVkFormat();
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
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

    m_ServiceLocator->m_RenderPasses.emplace(ForwardPass, m_ForwardRenderPass.get());

    RenderPass::CreateInfo shadowPassInfo;
    {
        auto& depth = shadowPassInfo.m_Attachments.emplace_back(); // 0
        depth = {};
        depth.format = VK_FORMAT_D32_SFLOAT; // TODO: this limits the shadow map format to D32_SFLOAT
        depth.samples = VK_SAMPLE_COUNT_1_BIT;
        depth.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        VkAttachmentReference ref = {};
        ref.attachment = 0; // depth
        ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        auto& subpass = shadowPassInfo.m_Subpasses.emplace_back(); // 0
        subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0; // No color attachments as we only want the depth buffer
        subpass.pDepthStencilAttachment = &ref;

        auto& dependency = shadowPassInfo.m_SubpassDependencies.emplace_back();
        dependency = {};
        dependency.dependencyFlags = 0;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
    }

    m_ShadowRenderPass = std::make_unique<RenderPass>(*m_ServiceLocator, shadowPassInfo);
    m_ServiceLocator->m_RenderPasses.emplace(ShadowPass, m_ShadowRenderPass.get());
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
    glm::mat4 transl = glm::translate(glm::identity<glm::mat4>(), glm::vec3(1.0f, 2.0f, 3.0f));
    glm::mat4 perspTest = glm::perspective(45.0f, 1.0f, 0.1f, 15.0f);

    glm::vec4 testV(0.0f, 0.0f, 5.0f, 1.0f);

    glm::vec4 fin = perspTest * testV;

    printf("Loading assets\n");

    auto& transferQueue = m_LogicalDevice->GetCommandQueue(ETransferQueue);

    m_TestShadowMap = std::make_unique<CubeShadowMap>(*m_ServiceLocator);
    auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/Sponza/Sponza.gltf");
    //auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/Tests/NormalTangentMirrorTest.gltf");
    //auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/CrowdKing/JL.gltf");
    //auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/Lantern/Lantern.gltf");
    //auto res = m_ModelManager->LoadGltf("../../../../Assets/GLTF/Tests/TwoSidedPlane.gltf");

    auto cubeRes = m_ModelManager->LoadGltf("../../../../Assets/Models/Cube.gltf");
    m_Sponza = res->GetScene();

    m_Light = m_Sponza->AddPointLight();
    //m_Light->SetPosition(glm::vec3(19.0f, 16.0f, 0.0f));

    //m_Light1 = m_Sponza->AddPointLight();
    //m_Light1->SetPosition(glm::vec3(5.0f, 2.0f, 0.0f));

    m_Camera = std::make_unique<Camera>();
    m_Camera->SetPosition(glm::vec3(0.0f, 0.0f, -0.0f));
    m_Camera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    m_Camera->SetAspectRatio(m_Window->GetAspectRatio());
    m_Camera->SetFarClipDistance(150.0f);
    m_Sponza->m_ActiveCamera = m_Camera.get();

    m_Sponza->m_StaticMeshes[0]->m_Transform->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    m_Sponza->m_StaticMeshes[0]->m_Transform->SetPosition(glm::vec3(-0.0f, 0.0f, 0.0f));
    //m_Sponza->m_StaticMeshes[0]->m_Transform->SetScale(glm::vec3(0.02f));
    //m_Sponza->m_StaticMeshes[0]->m_Transform->SetScale(glm::vec3(-1.0f, 40.0f, 70.0f));

    m_Sponza->m_StaticMeshes.push_back(std::make_unique<StaticMesh>());
    m_DebugCube = m_Sponza->m_StaticMeshes.back().get();
    m_Sponza->m_StaticMeshes.push_back(std::make_unique<StaticMesh>());
    m_DebugCube1 = m_Sponza->m_StaticMeshes.back().get();

    m_DebugCube1->m_Enabled = false;

    m_DebugCube->SetMesh(cubeRes->GetMesh());
    m_DebugCube1->SetMesh(cubeRes->GetMesh());

    m_DebugCube->m_Transform->SetScale(glm::vec3(0.1f, 0.1f, 0.1f));
    m_DebugCube1->m_Transform->SetScale(glm::vec3(10.0f, 10.0f, 10.0f));

    transferQueue.Flush();


    printf("All assets loaded.\n");
}

krt::Semaphore krt::Application::DrawFrame(krt::Semaphore a_LastFrameSem)
{
    auto imageAvailableSem = m_SemaphoreAllocator->GetSemaphore();
    auto drawFinishedSem = m_SemaphoreAllocator->GetSemaphore();

    auto frameInfo = m_Window->GetNextFrameInfo(imageAvailableSem);

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

    commandBuffer.BeginRenderPass(*m_ForwardRenderPass, *frameInfo.m_FrameBuffer, m_Window->GetScreenRenderArea());
    commandBuffer.SetScissorRect(scissor);
    commandBuffer.SetViewport(viewport);
    commandBuffer.BindPipeline(*m_GraphicsPipeline);

    glm::mat4 cameraMatrix = m_Camera->GetCameraMatrix();

    SemaphoreWait semWait = GenerateShadowMaps();

    //auto signalSem = m_SemaphoreAllocator->GetSemaphore();

    if (a_LastFrameSem)
    {
        //commandBuffer.AddWaitSemaphore(a_LastFrameSem, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    commandBuffer.AddWaitSemaphore(semWait.m_Semaphore, semWait.m_StageFlags);
    commandBuffer.SetDescriptorSet(m_Sponza->GetLightsDescriptorSet(semWait), 1);    
    commandBuffer.AddWaitSemaphore(semWait.m_Semaphore, semWait.m_StageFlags);
    //commandBuffer.AddSignalSemaphore(signalSem);

    for (auto& mesh : m_Sponza->m_StaticMeshes)
    {
        if (!mesh->m_Enabled)
            continue;

        auto& m = *mesh;
        glm::mat4 mvp;

        Mats mats;

        mats.m_MVP = cameraMatrix * mesh->m_Transform->GetTransformationMatrix();
        mats.m_World = mesh->m_Transform->GetTransformationMatrix();

        mvp = cameraMatrix * mesh->m_Transform->GetTransformationMatrix();
        commandBuffer.PushConstant(mats, 0);

        for (auto& primitive : m->m_Primitives)
        {            
            commandBuffer.SetVertexBuffer(*primitive.m_Positions, 0);
            commandBuffer.SetVertexBuffer(*primitive.m_TexCoords, 1);
            commandBuffer.SetVertexBuffer(*primitive.m_VertexColors, 2);
            commandBuffer.SetVertexBuffer(*primitive.m_Normals, 3);          
            commandBuffer.SetVertexBuffer(*primitive.m_Tangents, 4);

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
    commandBuffer.AddWaitSemaphore(imageAvailableSem, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    commandBuffer.Submit();

    m_ImGui->Display(frameInfo.m_FrameIndex, { drawFinishedSem });

    auto& presentQueue = m_ServiceLocator->m_LogicalDevice->GetCommandQueue(EPresentQueue);
    std::vector<Semaphore> presentSemaphores;
    presentSemaphores.push_back(drawFinishedSem);

    // Can do all ImGui after the present call, since at this point the CPU is already waiting for the GPU due to the lack of good management on my end.
    auto meshTransform = m_Sponza->m_StaticMeshes[0]->m_Transform.get();
    m_Window->Present(frameInfo.m_FrameIndex, presentQueue, presentSemaphores);

    glm::vec3 p = meshTransform->GetPosition();
    glm::vec3 r = meshTransform->GetRotationEuler();
    glm::vec3 s = meshTransform->GetScale();
    glm::vec3 v;
    glm::vec3 c;


    ImGui::Begin("Debug");
    ImGui::DragFloat3("Model Position", &p[0]);
    ImGui::DragFloat3("Model Rotation", &r[0]);
    ImGui::DragFloat3("Model Scale", &s[0]);
    v = m_Light->GetPosition();
    c = m_Light->GetColor();
    ImGui::DragFloat3("Light Position", &v[0]);
    ImGui::ColorEdit3("Light Color", &c[0], ImGuiColorEditFlags_Float);
    m_Light->SetColor(c);
    m_Light->SetPosition(v);

    if (std::abs(r.y) > 90.0f && std::abs(meshTransform->GetRotationEuler().y) < 90.0f)
    {
        //r.x += 180.0f;
        //r.z += 180.0f;
        r.y += 180.0f;
    }

    auto dif = r - meshTransform->GetRotationEuler();

    auto difQuat = glm::quat(glm::radians(dif));

    auto newRot = difQuat * meshTransform->GetRotationQuat();

    ImGui::End();
    meshTransform->SetPosition(p);
    meshTransform->SetRotation(newRot);
    meshTransform->SetScale(s);

    m_DebugCube->m_Transform->SetPosition(m_Light->GetPosition());

    //presentQueue.Flush();

    return drawFinishedSem;
}

void krt::Application::Cleanup()
{
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

krt::SemaphoreWait krt::Application::GenerateShadowMaps()
{
    auto& cmdBuffer = m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetSingleUseCommandBuffer();

    cmdBuffer.Begin();
    auto depthViews = m_TestShadowMap->GetDepthViews();

    auto& light = m_Light;

    auto& fbs = light->GetFramebuffers();



    for (int i = 0; i < 6; i++)
    {
        VkRect2D renderArea = {};
        renderArea.offset.x = 0;
        renderArea.offset.y = 0;
        renderArea.extent.width = 2048;
        renderArea.extent.height = 2048;

        cmdBuffer.BeginRenderPass(*m_ShadowRenderPass, *fbs[i], renderArea);
        cmdBuffer.SetScissorRect(renderArea);
        cmdBuffer.SetViewport(VkViewport{ 0.0f, 0.0f, 2048.0f, 2048.0f, 0.0f, 1.0f });
        cmdBuffer.BindPipeline(*m_ShadowPipeline);


        static const glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 20.0f);
        static const glm::vec3 lookDir[6] = {
            glm::vec3(1.0f,  0.0f,  0.0f),
            glm::vec3(-1.0f,  0.0f,  0.0f),
            glm::vec3( 0.0f, -1.0f,  0.0f),
            glm::vec3( 0.0f, 1.0f,  0.0f),
            glm::vec3( 0.0f,  0.0f, 1.0f),
            glm::vec3( 0.0f,  0.0f, -1.0f)
        };

        // TODO: This will likely fuck it up, particularly when up isn't (0.0f, 1.0f, 0.0f)
        static const glm::vec3 up[6] = {
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  0.0f,  1.0f),
            glm::vec3(0.0f,  0.0f,  -1.0f),
            glm::vec3(0.0f,  1.0f,  0.0f),
            glm::vec3(0.0f,  1.0f,  0.0f)
        };

        auto eye = light->GetPosition();
        auto center = eye + lookDir[i];
        auto lookat = glm::lookAt(eye, center, up[i]);

        glm::mat4 cameraMatrix = proj * lookat;

        for (auto& mesh : m_Sponza->m_StaticMeshes)
        {
            if (!mesh->m_Enabled)
                continue;
            if (m_DebugCube == mesh.get())
            {
                continue;
            }
            auto& m = *mesh;
            glm::mat4 mvp = cameraMatrix * mesh->m_Transform->GetTransformationMatrix();

            cmdBuffer.PushConstant(mvp, 0);

            for (auto& primitive : m->m_Primitives)
            {
                cmdBuffer.SetVertexBuffer(*primitive.m_Positions, 0);

                if (primitive.m_IndexBuffer)
                {
                    cmdBuffer.SetIndexBuffer(*primitive.m_IndexBuffer);
                    cmdBuffer.DrawIndexed(primitive.m_IndexBuffer->GetElementCount());
                }
                else
                {
                    cmdBuffer.Draw(primitive.m_Positions->GetElementCount());
                }
            }
        }

        cmdBuffer.EndRenderPass();
    }

    auto sem = m_ServiceLocator->m_SemaphoreAllocator->GetSemaphore();
    cmdBuffer.AddSignalSemaphore(sem);
    cmdBuffer.Submit();
    light->GetShadowMap().TransitionLayoutToShaderRead(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    SemaphoreWait semWait;
    semWait.m_Semaphore = sem;
    semWait.m_StageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    //m_LogicalDevice->GetCommandQueue(EGraphicsQueue).Flush();

    return semWait;
}

void krt::Application::InitializeImGui()
{
    m_ImGui = std::make_unique<VkImGui>(*m_ServiceLocator, *m_ForwardRenderPass);
}


void krt::Application::ProcessInput()
{
    auto cameraRotation = glm::mat4_cast(m_Camera->GetRotationQuat());

    auto forward4 = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f) * cameraRotation * 1.0f;
    auto right4 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * cameraRotation * 1.0f;

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


    /*if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_UP))
    {
        m_Camera->Rotate(glm::vec3(30.0f * 0.0f / 60.0f, 0.0f, 30.0f * 1.0f / 60.0f));
    }
    else if (glfwGetKey(m_Window->GetGLFWwindow(), GLFW_KEY_DOWN))
    {
        m_Camera->Rotate(glm::vec3(-30.0f * 0.0f / 60.0f, 0.0f, -30.0f * 1.0f / 60.0f));
    };*/
}

void krt::Application::FocusCallback(GLFWwindow*, int a_Focus)
{
    if (a_Focus == GLFW_TRUE)
        g_Application->m_InFocus = true;
    else
        g_Application->m_InFocus = false;
}
