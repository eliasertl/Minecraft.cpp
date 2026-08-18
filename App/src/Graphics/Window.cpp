#include "Window.h"
#include "Core/Logger.h"

#include <glfw/glfw3.h>

namespace Minecraft::Graphics {
    bool isGlfwInitialized = false;

    Window::Window(WindowProps props){
        if(!isGlfwInitialized){
            LOG_INFO("Initializing GLFW");
            glfwInit();
            isGlfwInitialized = true;
        }

        LOG_INFO("Creating Window {} ({}x{})", props.title, props.width, props.height);
        m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
    }

    Window::~Window(){
        LOG_INFO("Destroying Window");
        glfwDestroyWindow(m_Window);
    }

    bool Window::ShouldClose(){
        return glfwWindowShouldClose(m_Window);
    }

    void Window::Maximize(){
        glfwMaximizeWindow(m_Window);
    }

    void Window::Update(){
        glfwPollEvents();
    }
}