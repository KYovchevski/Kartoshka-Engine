#pragma once

#include <memory>
#include <vector>

namespace krt
{
    class StaticMesh;
}

namespace krt
{

    class Scene
    {
    public:
        std::vector<std::unique_ptr<StaticMesh>> m_StaticMeshes;
    };

}
