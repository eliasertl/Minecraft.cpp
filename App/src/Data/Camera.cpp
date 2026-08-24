#include "Camera.h"
#include "Common/Input.h"
#include "Core/Logger.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Minecraft::Data
{
    Camera::Camera(Graphics::Window &window)
        : m_Window(window)
    {
        UpdateViewProjection();
    }

    Camera::~Camera()
    {
    }

    void Camera::Update(float deltaTime)
    {
        if (m_Window.GetWidth() != m_Width || m_Window.GetHeight() != m_Height)
        {
            SetViewSize(static_cast<float>(m_Window.GetWidth()), static_cast<float>(m_Window.GetHeight()));
        }

        bool updateViewProjection = false;

        glm::vec3 movement = glm::vec3(0.0f);
        if (Input::IsKeyPressed(KeyCode::W))
            movement.z -= m_MovementSpeed;
        if (Input::IsKeyPressed(KeyCode::S))
            movement.z += m_MovementSpeed;
        if (Input::IsKeyPressed(KeyCode::A))
            movement.x -= m_MovementSpeed;
        if (Input::IsKeyPressed(KeyCode::D))
            movement.x += m_MovementSpeed;
        if (Input::IsKeyPressed(KeyCode::Space))
            movement.y += m_MovementSpeed * 0.7f;
        if (Input::IsKeyPressed(KeyCode::LeftCtrl))
            movement.y -= m_MovementSpeed * 0.7f;

        glm::vec2 mouseDelta = glm::vec2(0.0f);
        if(Input::IsMouseButtonPressed(MouseButton::Right))
        {
            mouseDelta = Input::GetMouseDelta();
        }
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            m_Yaw -= static_cast<float>(mouseDelta.x) * m_RotationSpeed;
            m_Pitch -= static_cast<float>(mouseDelta.y) * m_RotationSpeed;

            m_Pitch = glm::clamp(
                m_Pitch,
                glm::radians(-89.0f),
                glm::radians(89.0f));

            glm::mat4 rotation(1.0f);

            rotation = glm::rotate(
                rotation,
                m_Yaw,
                glm::vec3(0.0f, 1.0f, 0.0f));

            rotation = glm::rotate(
                rotation,
                m_Pitch,
                glm::vec3(1.0f, 0.0f, 0.0f));

            glm::vec3 position = glm::vec3(m_Transform[3]);

            m_Transform = rotation;
            m_Transform[3] = glm::vec4(position, 1.0f);

            updateViewProjection = true;
        }

        if (movement.x != 0.0f || movement.y != 0.0f || movement.z != 0.0f)
        {
            m_Transform = glm::translate(m_Transform, movement * deltaTime);
            updateViewProjection = true;
        }

        if (updateViewProjection)
            UpdateViewProjection();
    }

    void Camera::UpdateViewProjection()
    {
        if (m_Width == 0 || m_Height == 0)
        {
            LOG_WARN("Camera view size is zero, skipping view projection update");
            return;
        }
        m_ViewProjection = glm::perspective(m_FOV, static_cast<float>(m_Width) / static_cast<float>(m_Height), m_NearPlane, m_FarPlane) * glm::inverse(m_Transform);
    }
}