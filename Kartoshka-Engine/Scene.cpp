#include "Scene.h"

#include "LogicalDevice.h"
#include "ServiceLocator.h"
#include "GraphicsPipeline.h"
#include "DescriptorSet.h"
#include "PointLight.h"
#include "StaticMesh.h"
#include "Transform.h"
#include "Camera.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Sampler.h"

#include "CubeShadowMap.h"

#include <glm/vec3.hpp>

krt::Scene::Scene(ServiceLocator& a_Services)
    : m_Services(a_Services)
    , m_LightsDescriptorSetDirty(true)
{
    m_LightsDescriptorSet = m_Services.m_GraphicsPipelines[Forward]->CreateDescriptorSet(1, { EGraphicsQueue });

    Sampler::CreateInfo smpInfo;
    smpInfo->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    smpInfo->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    smpInfo->addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    smpInfo->borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    smpInfo->compareEnable = VK_TRUE;
    smpInfo->compareOp = VK_COMPARE_OP_LESS;
    m_ShadowMapSampler = std::make_unique<Sampler>(m_Services, smpInfo);
}

krt::Scene::~Scene()
{
}

krt::PointLight* krt::Scene::AddPointLight()
{
    std::unique_ptr<CubeShadowMap> cubeMap = std::make_unique<CubeShadowMap>(m_Services, 2048);


    std::array<std::unique_ptr<Framebuffer>, 6> framebuffers;
    auto depthViews = cubeMap->GetDepthViews();


    for (size_t i = 0; i < depthViews.size(); i++)
    {
        auto& framebuffer = framebuffers[i];
        framebuffer = m_Services.m_RenderPasses[ShadowPass]->CreateFramebuffer();
        framebuffer->SetSize(glm::uvec2(2048, 2048));
        framebuffer->AddImageView(depthViews[i], 0);
    }

    cubeMap->TransitionLayoutToShaderRead(VK_IMAGE_LAYOUT_UNDEFINED, 0);
    auto& newLight = m_PointLights.emplace_back(std::make_unique<PointLight>(*this, cubeMap, framebuffers));
    m_LightsDescriptorSetDirty = true;


    // TODO: temporary solution, needs to be redone when shadowmapping is enabled for more than one lights
    m_LightsDescriptorSet->SetTexture(m_PointLights.front()->GetShadowMap(), 1);
    m_LightsDescriptorSet->SetSampler(*m_ShadowMapSampler, 2);

    return newLight.get();
}

krt::DescriptorSet& krt::Scene::GetLightsDescriptorSet(SemaphoreWait& a_WaitSemaphore) const
{
    if (m_LightsDescriptorSetDirty)
    {
        struct PointLight
        {
            glm::vec3 m_Position;
            float __Padding;
            glm::vec3 m_Color;
            float __Padding1;
        };

        std::vector<PointLight> lights;
        lights.reserve(m_PointLights.size());

        for (auto& pointLight : m_PointLights)
        {
            auto& light = lights.emplace_back();
            light.m_Position = pointLight->GetPosition();
            light.m_Color = pointLight->GetColor();            
        }

        struct Header
        {
            uint32_t m_NumLights;
            float m_FarClip;
            float m_NearClip;
            uint32_t _Padding[1];
        } header;

        header.m_NumLights = static_cast<uint32_t>(m_PointLights.size());
        header.m_FarClip = 20.0f;
        header.m_NearClip = 0.01f;

        m_LightsDescriptorSetDirty = false;
        a_WaitSemaphore = m_LightsDescriptorSet->SetStorageBuffer(header, lights, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    return *m_LightsDescriptorSet;
}

void krt::Scene::MakeLightsDirty()
{
    m_LightsDescriptorSetDirty = true;
}
