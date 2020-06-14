#include "Transform.h"

krt::Transform::Transform()
    : m_Position(0.0f)
    , m_Rotation(1.0f, 0.0f, 0.0f, 0.0f)
    , m_Scale(1.0f)
    , m_MatrixDirty(true)
    , m_TransformationMatrix(1.0f)
{

}

krt::Transform::~Transform()
{
}

void krt::Transform::SetPosition(const glm::vec3& a_NewPosition)
{
    m_Position = a_NewPosition;
    MakeDirty();
}

void krt::Transform::SetRotation(const glm::quat& a_Quaternion)
{
    m_Rotation = a_Quaternion;
    MakeDirty();
}

void krt::Transform::SetRotation(const glm::vec3& a_EulerDegrees)
{
    SetRotation(glm::quat(glm::radians(a_EulerDegrees)));
}

void krt::Transform::SetRotation(const glm::vec3& a_Pivot, float a_Degrees)
{
    SetRotation(a_Pivot * a_Degrees);
}

void krt::Transform::SetScale(const glm::vec3& a_Scale)
{
    m_Scale = a_Scale;
    MakeDirty();
}

void krt::Transform::Move(const glm::vec3& a_Movement)
{
    SetPosition(GetPosition() + a_Movement);
}

void krt::Transform::Rotate(const glm::quat& a_Quaternion)
{
    SetRotation(GetRotationQuat() * a_Quaternion);
}

void krt::Transform::Rotate(const glm::vec3& a_EulerDegrees)
{
    Rotate(glm::quat(glm::radians(a_EulerDegrees)));
}

void krt::Transform::Rotate(const glm::vec3& a_Pivot, float a_Degrees)
{
    Rotate(a_Pivot * a_Degrees);
}

void krt::Transform::ScaleUp(const glm::vec3 a_Scale)
{
    SetScale(GetScale() + a_Scale);
}

void krt::Transform::TransformBy(const Transform& a_Other)
{
    SetPosition(glm::vec4(GetPosition(), 1.0f) * static_cast<glm::mat4>(a_Other));
    SetRotation(GetRotationQuat() * a_Other.GetRotationQuat());
    SetScale(GetScale() * a_Other.GetScale());
}

void krt::Transform::CopyTransform(const Transform& a_Other)
{
    SetPosition(a_Other.GetPosition());
    SetRotation(a_Other.GetRotationQuat());
    SetScale(a_Other.GetScale());
}

const glm::vec3& krt::Transform::GetPosition() const
{
    return m_Position;
}

const glm::quat& krt::Transform::GetRotationQuat() const
{
    return m_Rotation;
}

glm::vec3 krt::Transform::GetRotationEuler() const
{
    return glm::degrees(glm::eulerAngles(m_Rotation));
}

const glm::vec3& krt::Transform::GetScale() const
{
    return m_Scale;
}

glm::mat4 krt::Transform::GetTransformationMatrix() const
{
    UpdateMatrix();
    return m_TransformationMatrix;
}

krt::Transform::operator glm::mat<4, 4, float, glm::defaultp>() const
{
    return GetTransformationMatrix();
}

void krt::Transform::MakeDirty()
{
    m_MatrixDirty = true;
}

void krt::Transform::UpdateMatrix() const
{
    if (!m_MatrixDirty)
        return;

    m_TransformationMatrix = glm::mat4(1.0f);

    glm::mat4 rotation = glm::mat4_cast(m_Rotation);

    m_TransformationMatrix = glm::translate(m_TransformationMatrix, m_Position);
    m_TransformationMatrix = m_TransformationMatrix * rotation;
    m_TransformationMatrix = glm::scale(m_TransformationMatrix, m_Scale);

    m_MatrixDirty = false;

}
