#pragma once

#include <memory>
#include <vector>

namespace krt
{
    struct SemaphoreWait;
    struct ServiceLocator;
    class StaticMesh;
    class DescriptorSet;
    class PointLight;
    class Camera;
}

namespace krt
{
    class Scene
    {
    public:
        Scene(ServiceLocator& a_Services);
        ~Scene();

        std::vector<std::unique_ptr<StaticMesh>> m_StaticMeshes;

        PointLight* AddPointLight();

        DescriptorSet& GetLightsDescriptorSet(SemaphoreWait& a_WaitSemaphore) const;
        void MakeLightsDirty();

        Camera* m_ActiveCamera;
    private:

        ServiceLocator& m_Services;

        std::vector<std::unique_ptr<PointLight>> m_PointLights;
        mutable std::unique_ptr<DescriptorSet> m_LightsDescriptorSet;
        mutable bool m_LightsDescriptorSetDirty;


    };

}
