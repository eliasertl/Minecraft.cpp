#pragma once

#include <webgpu/webgpu_cpp.h>
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

        wgpu::Surface CreateSurface(wgpu::Instance instance);
    
    private:
        GLFWwindow* m_Window;
    };
}