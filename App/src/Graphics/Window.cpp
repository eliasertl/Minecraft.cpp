#include "Window.h"
#include "Core/Logger.h"

#include <glfw/glfw3.h>
#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <Windows.h>
    #include <glfw/glfw3native.h>
#endif

namespace Minecraft::Graphics
{
    bool isGlfwInitialized = false;

    Window::Window(WindowProps props)
    {
        if (!isGlfwInitialized)
        {
            LOG_INFO("Initializing GLFW");
            if (!glfwInit())
            {
                LOG_ERROR("Failed to initialize GLFW");
                return;
            }
            isGlfwInitialized = true;
        }

        LOG_INFO("Creating Window {} ({}x{})", props.title, props.width, props.height);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, this); 
        m_Width = props.width;
        m_Height = props.height;

        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            win->m_Width = width;
            win->m_Height = height;
        });
    }

    Window::~Window()
    {
        LOG_INFO("Destroying Window");
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(m_Window);
    }

    void Window::Maximize()
    {
        glfwMaximizeWindow(m_Window);
    }

    void Window::SetCursorMode(WindowCursorMode mode)
    {
        switch (mode)
        {
            case WindowCursorMode::Normal:
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
            case WindowCursorMode::Hidden:
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                break;
            case WindowCursorMode::Disabled:
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
        }
    }

    void Window::Update()
    {
        m_DeltaTime = glfwGetTime() - m_LastFrameTime;
        m_LastFrameTime = glfwGetTime();
        glfwPollEvents();
    }

    wgpu::Surface Window::CreateSurface(wgpu::Instance instance)
    {
        LOG_INFO("Creating Surface for Window");
#ifdef _WIN32
        wgpu::SurfaceSourceWindowsHWND surfaceSource = {};
        surfaceSource.hwnd = glfwGetWin32Window(m_Window);
        surfaceSource.hinstance = GetModuleHandle(nullptr);

        wgpu::SurfaceDescriptor desc = {};
        desc.nextInChain =
            reinterpret_cast<wgpu::ChainedStruct *>(&surfaceSource);

        return instance.CreateSurface(&desc).Get();
#endif
    }
}