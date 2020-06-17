#include "StaticMesh.h"

#include "Transform.h"

krt::StaticMesh::StaticMesh()
    : m_Mesh(nullptr)
    , m_Transform(std::make_unique<Transform>())
{
}
