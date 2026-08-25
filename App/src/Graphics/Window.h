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

    enum class WindowCursorMode {
        Normal = 0,
        Hidden = 1,
        Disabled = 2
    };

    class Window
    {
    public:
        Window(WindowProps props = {});
        ~Window();

        bool ShouldClose();

        void Update();
        void Maximize();

        void SetCursorMode(WindowCursorMode mode);

        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }
        inline GLFWwindow* GetNativeWindow() const { return m_Window; }
        inline double GetDeltaTime() const { return m_DeltaTime; }

        wgpu::Surface CreateSurface(wgpu::Instance instance);
    
    private:
        GLFWwindow* m_Window;
        uint32_t m_Width, m_Height;
        double m_LastFrameTime;
        double m_DeltaTime;
    };
}