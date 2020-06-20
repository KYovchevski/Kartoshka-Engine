#include "Camera.h"

#include "glm/gtx/quaternion.hpp"

krt::Camera::Camera(float a_FoV, float a_AspectRatio, float a_NearClip, float a_FarClip)
    : m_Rotation(1.0f, 0.0f, 0.0f, 0.0f)
    , m_MatrixDirty(true)
    , m_ViewProjectionMatrix(1.0f)
    , m_FieldOfView(a_FoV)
    , m_AspectRatio(a_AspectRatio)
    , m_NearClipPlane(a_NearClip)
    , m_FarClipPlane(a_FarClip)
    , m_Position(0.0f)
{
}

krt::Camera::~Camera()
{
}

void krt::Camera::SetPosition(const glm::vec3& a_NewPosition)
{
    m_Position = a_NewPosition;
    MakeViewDirty();
}

void krt::Camera::SetRotation(const glm::quat& a_Quaternion)
{
    m_Rotation = a_Quaternion;
    MakeViewDirty();
}

void krt::Camera::SetRotation(const glm::vec3& a_EulerDegrees)
{
    SetRotation(glm::quat(glm::radians(a_EulerDegrees)));
}

void krt::Camera::SetRotation(const glm::vec3& a_Pivot, float a_Angle)
{
    SetRotation(a_Pivot * a_Angle);
}

void krt::Camera::Move(const glm::vec3& a_Movement)
{
    SetPosition(GetPosition() + a_Movement);
}

void krt::Camera::Rotate(const glm::quat& a_Quaternion)
{
    SetRotation(GetRotationQuat() * a_Quaternion);
}

void krt::Camera::Rotate(const glm::vec3& a_EulerDegrees)
{
    Rotate(glm::quat(glm::radians(a_EulerDegrees)));
}

void krt::Camera::Rotate(const glm::vec3& a_Pivot, float a_Degrees)
{
    Rotate(a_Pivot * a_Degrees);
}

void krt::Camera::SetFieldOfView(float a_NewFoV)
{
    m_FieldOfView = a_NewFoV;
    MakeProjDirty();
}

void krt::Camera::SetAspectRatio(float a_NewAspectRatio)
{
    m_AspectRatio = a_NewAspectRatio;
    MakeProjDirty();
}

void krt::Camera::SetNearClipDistance(float a_NewNearClip)
{
    m_NearClipPlane = a_NewNearClip;
    MakeProjDirty();
}

void krt::Camera::SetFarClipDistance(float a_NewFarClip)
{
    m_FarClipPlane = a_NewFarClip;
    MakeProjDirty();
}

const glm::vec3& krt::Camera::GetPosition() const
{
    return m_Position;
}

const glm::quat& krt::Camera::GetRotationQuat() const
{
    return m_Rotation;
}

glm::vec3 krt::Camera::GetRotationEuler() const
{
    return glm::degrees(glm::eulerAngles(m_Rotation));
}

float krt::Camera::GetFieldOfView() const
{
    return m_FieldOfView;
}

float krt::Camera::GetNearClipDistance() const
{
    return m_NearClipPlane;
}

float krt::Camera::GetFarClipDistance() const
{
    return m_FarClipPlane;
}


const glm::mat4 krt::Camera::GetCameraMatrix() const
{
    UpdateMatrix();
    return m_ViewProjectionMatrix;
}

const glm::mat4 krt::Camera::GetViewMatrix() const
{
    UpdateViewMatrix();
    return m_ViewMatrix;
}

void krt::Camera::MakeProjDirty()
{
    m_ProjectionDirty = true;
    MakeDirty();
}

const glm::mat4 krt::Camera::GetProjectionMatrix() const
{
    UpdateProjectionMatrix();
    return m_ProjectionMatrix;
}

void krt::Camera::MakeViewDirty()
{
    m_ViewDirty = true;
    MakeDirty();
}

void krt::Camera::MakeDirty()
{
    m_MatrixDirty = true;
}

void krt::Camera::UpdateMatrix() const
{
    if (!m_MatrixDirty)
        return;

    m_ViewProjectionMatrix = GetProjectionMatrix() * GetViewMatrix();
    m_MatrixDirty = false;
}

void krt::Camera::UpdateViewMatrix() const
{
    if (m_ViewDirty)
    {
        auto forward = glm::vec3(0.0f, 0.0f, 1.0f) * m_Rotation;
        auto up = glm::vec3(0.0f, 1.0f, 0.0f) * m_Rotation;
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, up);

        m_ViewDirty = false;
    }
}

void krt::Camera::UpdateProjectionMatrix() const
{
    if (m_ProjectionDirty)
    {
        m_ProjectionMatrix = glm::perspective(glm::radians(m_FieldOfView), m_AspectRatio, m_NearClipPlane, m_FarClipPlane);

        m_ProjectionDirty = false;
    }
}
