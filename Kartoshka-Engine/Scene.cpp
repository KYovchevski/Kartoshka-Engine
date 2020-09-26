#include "Scene.h"

#include "LogicalDevice.h"
#include "ServiceLocator.h"
#include "GraphicsPipeline.h"
#include "DescriptorSet.h"
#include "PointLight.h"
#include "StaticMesh.h"
#include "Transform.h"
#include "Camera.h"

#include <glm/vec3.hpp>

krt::Scene::Scene(ServiceLocator& a_Services)
    : m_Services(a_Services)
    , m_LightsDescriptorSetDirty(true)
{
    m_LightsDescriptorSet = m_Services.m_GraphicsPipelines["Scene"]->CreateDescriptorSet(1, { EGraphicsQueue });
}

krt::Scene::~Scene()
{
}

krt::PointLight* krt::Scene::AddPointLight()
{
    auto& newLight = m_PointLights.emplace_back(std::make_unique<PointLight>(*this));
    m_LightsDescriptorSetDirty = true;

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
            glm::ivec3 _padding;
        } header;

        header.m_NumLights = static_cast<uint32_t>(m_PointLights.size());

        m_LightsDescriptorSetDirty = false;
        a_WaitSemaphore = m_LightsDescriptorSet->SetStorageBuffer(header, lights, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    return *m_LightsDescriptorSet;
}

void krt::Scene::MakeLightsDirty()
{
    m_LightsDescriptorSetDirty = true;
}
