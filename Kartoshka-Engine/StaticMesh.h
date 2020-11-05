#pragma once

#include <memory>

namespace krt
{
    struct Mesh;
    class Transform;
}

namespace krt
{
    class StaticMesh
    {
    public:

        StaticMesh();

        StaticMesh(StaticMesh&&) = delete;
        StaticMesh& operator=(StaticMesh&&) = delete;

        const Mesh* operator->() { return m_Mesh.get(); }

        std::unique_ptr<Transform> m_Transform;

        void SetMesh(std::shared_ptr<Mesh> a_Mesh) { m_Mesh = a_Mesh; }

        bool m_Enabled;

    private:
        std::shared_ptr<Mesh> m_Mesh;
    };
}

