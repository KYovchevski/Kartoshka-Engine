#pragma once
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include "glm/ext/quaternion_float.hpp"

namespace krt
{
    class Camera
    {
    public:
        Camera(float a_FoV = 45.0f, float a_AspectRatio = 1.0f, float a_NearClip = 0.01f, float a_FarClip = 15.0f);
        ~Camera();

        void SetPosition(const glm::vec3& a_NewPosition);
        void SetRotation(const glm::quat& a_Quaternion);
        void SetRotation(const glm::vec3& a_EulerDegrees);
        void SetRotation(const glm::vec3& a_Pivot, float a_Angle);

        void Move(const glm::vec3& a_Movement);
        void Rotate(const glm::quat& a_Quaternion);
        void Rotate(const glm::vec3& a_EulerDegrees);
        void Rotate(const glm::vec3& a_Pivot, float a_Degrees);

        void SetFieldOfView(float a_NewFoV);
        void SetAspectRatio(float a_NewAspectRatio);
        void SetNearClipDistance(float a_NewNearClip);
        void SetFarClipDistance(float a_NewFarClip);

        const glm::vec3& GetPosition() const;
        const glm::quat& GetRotationQuat() const;
        glm::vec3 GetRotationEuler() const;
        float GetFieldOfView() const;
        float GetNearClipDistance() const;
        float GetFarClipDistance() const;

        const glm::mat4 GetCameraMatrix() const;

    private:
        void MakeDirty();
        void UpdateMatrix() const;

        glm::vec3 m_Position;
        glm::quat m_Rotation;

        float m_FieldOfView;
        float m_AspectRatio;
        float m_NearClipPlane;
        float m_FarClipPlane;

        mutable glm::mat4 m_ViewProjectionMatrix;
        mutable bool m_MatrixDirty;
    };
}

