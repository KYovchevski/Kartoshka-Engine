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

krt::DescriptorSet& krt::Scene::GetLightsDescriptorSet() const
{
    if (m_LightsDescriptorSetDirty)
    {
        struct Light
        {
            glm::vec3 m_Position;
            float m_Padding;
            glm::vec3 m_Color;
        } light;

        glm::vec3 p = m_PointLights[0]->GetPosition();
        glm::vec4 viewPos = glm::vec4(p[0], p[1], p[2], 1.0f) * m_ActiveCamera->GetViewMatrix();

        light.m_Position = glm::vec3(viewPos[0], viewPos[1], viewPos[2]);
        //light.m_Position = p;
        light.m_Color = m_PointLights[0]->GetColor();

        m_LightsDescriptorSetDirty = false;
        m_LightsDescriptorSet->SetUniformBuffer(light, 0);
    }

    return *m_LightsDescriptorSet;
}

void krt::Scene::MakeLightsDirty()
{
    m_LightsDescriptorSetDirty = true;
}
