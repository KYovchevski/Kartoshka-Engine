#pragma once

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace krt
{
    class Scene;
}

namespace krt
{
    class PointLight
    {
    public:
        PointLight(Scene& a_Scene);

        void SetPosition(const glm::vec3& a_NewPosition);
        void SetColor(const glm::vec3& a_NewColor);

        const glm::vec3& GetPosition() const;
        const glm::vec3& GetColor() const;

    private:

        void UpdateSceneDescriptorSet();

        Scene* m_OwnerScene;

        glm::vec3 m_Position;
        glm::vec3 m_Color;
    };
}
