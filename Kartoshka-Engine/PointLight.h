#pragma once

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <memory>
#include <vector>
#include <array>

namespace krt
{
    class Scene;
    class Framebuffer;
    class CubeShadowMap;
}

namespace krt
{
    class PointLight
    {
    public:
        PointLight(Scene& a_Scene, std::unique_ptr<CubeShadowMap>& a_ShadowMap, std::array<std::unique_ptr<Framebuffer>, 6>& a_Framebuffers);

        void SetPosition(const glm::vec3& a_NewPosition);
        void SetColor(const glm::vec3& a_NewColor);

        float GetFarClipDistance() const;
        float GetNearClipDistance() const;
        const glm::vec3& GetPosition() const;
        const glm::vec3& GetColor() const;
        const std::array<std::unique_ptr<Framebuffer>, 6>& GetFramebuffers();
        CubeShadowMap& GetShadowMap();

    private:

        void UpdateSceneDescriptorSet();

        Scene* m_OwnerScene;

        glm::vec3 m_Position;
        glm::vec3 m_Color;

        std::array<std::unique_ptr<Framebuffer>, 6>     m_ShadowMapFramebuffers;
        std::unique_ptr<CubeShadowMap>                  m_ShadowMap;
    };
}
