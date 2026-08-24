#pragma once

#include "Data/KeyCodes.h"
#include "Data/MouseButtons.h"
#include "Graphics/Window.h"

#include <glm/glm.hpp>

namespace Minecraft::Common
{
    class Input
    {
    public:
        static void Init(Graphics::Window* window);
        static void Update();

        static bool IsKeyPressed(Data::KeyCode key);
        static bool IsKeyReleased(Data::KeyCode key);
        static bool IsKeyDown(Data::KeyCode key);

        static bool IsMouseButtonPressed(Data::MouseButton button);
        static bool IsMouseButtonReleased(Data::MouseButton button);
        static bool IsMouseButtonDown(Data::MouseButton button);

        static double GetMouseX();
        static double GetMouseY();
        static glm::vec2 GetMousePosition();

        static double GetMouseDeltaX();
        static double GetMouseDeltaY();
        static glm::vec2 GetMouseDelta();

    private:
        static Graphics::Window* s_Window;

        static double s_MouseX;
        static double s_MouseY;

        static double s_MouseDeltaX;
        static double s_MouseDeltaY;
    };
}

namespace Minecraft
{
    using namespace Common;
}