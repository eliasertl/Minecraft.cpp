#include "Input.h"

namespace Minecraft::Common
{
    Graphics::Window* Input::s_Window = nullptr;

    double Input::s_MouseX = 0.0;
    double Input::s_MouseY = 0.0;

    double Input::s_MouseDeltaX = 0.0;
    double Input::s_MouseDeltaY = 0.0;

    void Input::Init(Graphics::Window* window)
    {
        s_Window = window;

        glfwGetCursorPos(
            s_Window->GetNativeWindow(),
            &s_MouseX,
            &s_MouseY
        );
    }

    void Input::Update()
    {
        if (!s_Window)
            return;

        double newMouseX;
        double newMouseY;

        glfwGetCursorPos(
            s_Window->GetNativeWindow(),
            &newMouseX,
            &newMouseY
        );

        s_MouseDeltaX = newMouseX - s_MouseX;
        s_MouseDeltaY = newMouseY - s_MouseY;

        s_MouseX = newMouseX;
        s_MouseY = newMouseY;
    }

    bool Input::IsKeyPressed(Data::KeyCode key)
    {
        return glfwGetKey(
            s_Window->GetNativeWindow(),
            static_cast<int>(key)
        ) == GLFW_PRESS;
    }

    bool Input::IsKeyReleased(Data::KeyCode key)
    {
        return glfwGetKey(
            s_Window->GetNativeWindow(),
            static_cast<int>(key)
        ) == GLFW_RELEASE;
    }

    bool Input::IsKeyDown(Data::KeyCode key)
    {
        return glfwGetKey(
            s_Window->GetNativeWindow(),
            static_cast<int>(key)
        ) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonPressed(Data::MouseButton button)
    {
        return glfwGetMouseButton(
            s_Window->GetNativeWindow(),
            static_cast<int>(button)
        ) == GLFW_PRESS;
    }

    bool Input::IsMouseButtonReleased(Data::MouseButton button)
    {
        return glfwGetMouseButton(
            s_Window->GetNativeWindow(),
            static_cast<int>(button)
        ) == GLFW_RELEASE;
    }

    bool Input::IsMouseButtonDown(Data::MouseButton button)
    {
        return glfwGetMouseButton(
            s_Window->GetNativeWindow(),
            static_cast<int>(button)
        ) == GLFW_PRESS;
    }

    double Input::GetMouseX()
    {
        return s_MouseX;
    }

    double Input::GetMouseY()
    {
        return s_MouseY;
    }

    glm::vec2 Input::GetMousePosition()
    {
        return {
            static_cast<float>(s_MouseX),
            static_cast<float>(s_MouseY)
        };
    }

    double Input::GetMouseDeltaX()
    {
        return s_MouseDeltaX;
    }

    double Input::GetMouseDeltaY()
    {
        return s_MouseDeltaY;
    }

    glm::vec2 Input::GetMouseDelta()
    {
        return {
            static_cast<float>(s_MouseDeltaX),
            static_cast<float>(s_MouseDeltaY)
        };
    }
}