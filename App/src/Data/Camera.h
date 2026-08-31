#pragma once

#include "Graphics/Window.h"

#include <glm/glm.hpp>

namespace Minecraft::Data
{
    class Camera
    {
    public:
        Camera(Graphics::Window &window);
        ~Camera();

        void Update(float deltaTime);

        inline const glm::mat4 &GetViewProjection() const { return m_ViewProjection; }
        inline const glm::mat4 &GetTransform() const { return m_Transform; }

        inline void SetTransform(glm::mat4 transform) { m_Transform = transform; UpdateViewProjection(); }
        inline glm::vec3 GetPosition() const { return glm::vec3(m_Transform[3]); }


        inline void SetFOV(float fov) { m_FOV = fov; UpdateViewProjection(); }
        inline void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; UpdateViewProjection(); }
        inline void SetFarPlane(float farPlane) { m_FarPlane = farPlane; UpdateViewProjection(); }
        inline void SetViewSize(float width, float height) { m_Width = width; m_Height = height; UpdateViewProjection(); }
        inline void SetViewSize(glm::vec2 size) { SetViewSize(size.x, size.y); }

        inline void SetMovementSpeed(float speed) { m_MovementSpeed = speed; }
        inline void SetRotationSpeed(float speed) { m_RotationSpeed = speed; }

        inline float GetFOV() const { return m_FOV; }

        inline float GetNearPlane() const { return m_NearPlane; }
        inline float GetFarPlane() const { return m_FarPlane; }
        inline glm::vec2 GetViewSize() const { return glm::vec2(m_Width, m_Height); }

        inline float GetYaw() const { return m_Yaw; }
        inline float GetPitch() const { return m_Pitch; }

        inline float GetMovementSpeed() const { return m_MovementSpeed; }
        inline float GetRotationSpeed() const { return m_RotationSpeed; }

    private:
        void UpdateViewProjection();

    private:
        Graphics::Window &m_Window;

        glm::mat4 m_ViewProjection = glm::mat4(1.0f);
        glm::mat4 m_Transform = glm::mat4(1.0f);

        uint32_t m_Width = 1280, m_Height = 720;
        float m_FOV = glm::radians(45.0f);
        float m_NearPlane = 0.01f, m_FarPlane = 1000.0f;

        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;

        float m_MovementSpeed = 5.0f;
        float m_RotationSpeed = 0.0025f;
    };
}