#include "PointLight.h"
#include "Scene.h"

krt::PointLight::PointLight(Scene& a_Scene)
    : m_OwnerScene(&a_Scene)
    , m_Color(1.0f)
    , m_Position(0.0f)
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

void krt::PointLight::UpdateSceneDescriptorSet()
{
    m_OwnerScene->MakeLightsDirty();
}
