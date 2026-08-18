#pragma once

#include <string>

struct GLFWwindow;

namespace Minecraft::Graphics {
    struct WindowProps{
        int width = 900;
        int height = 600;
        std::string title = "Window";
    };

    class Window
    {
    public:
        Window(WindowProps props = {});
        ~Window();

        bool ShouldClose();

        void Update();
        void Maximize();
    
    private:
        GLFWwindow* m_Window;
    };
}