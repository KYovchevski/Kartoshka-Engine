#include "PointLight.h"
#include "Scene.h"
#include "CubeShadowMap.h"
#include "Framebuffer.h"

krt::PointLight::PointLight(Scene& a_Scene, std::unique_ptr<CubeShadowMap>& a_ShadowMap,
    std::array<std::unique_ptr<Framebuffer>, 6>& a_Framebuffers)
    : m_OwnerScene(&a_Scene)
    , m_Color(1.0f)
    , m_Position(0.0f)
    , m_ShadowMap(std::move(a_ShadowMap))
    , m_ShadowMapFramebuffers(std::move(a_Framebuffers))
{
}

void krt::PointLight::SetPosition(const glm::vec3& a_NewPosition)
{
    m_Position = a_NewPosition;
    UpdateSceneDescriptorSet();
}

void krt::PointLight::SetColor(const glm::vec3& a_NewColor)
{
    m_Color = a_NewColor;
    UpdateSceneDescriptorSet();
}

const glm::vec3& krt::PointLight::GetPosition() const
{
    return m_Position;
}

const glm::vec3& krt::PointLight::GetColor() const
{
    return m_Color;
}

const std::array<std::unique_ptr<krt::Framebuffer>, 6>& krt::PointLight::GetFramebuffers()
{
    return m_ShadowMapFramebuffers;
}

krt::CubeShadowMap& krt::PointLight::GetShadowMap()
{
    return *m_ShadowMap;
}

void krt::PointLight::UpdateSceneDescriptorSet()
{
    m_OwnerScene->MakeLightsDirty();
}
