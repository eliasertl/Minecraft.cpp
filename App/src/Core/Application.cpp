#include "Application.h"
#include "Logger.h"

#include <stdio.h>

namespace Minecraft::Core {
    Application::Application() {
        Log::Init();
        LOG_INFO("Creating Application");

        Graphics::WindowProps winProps;
        winProps.title = "Minecraft.cpp";
        m_Window = CreateScope<Graphics::Window>(winProps);

        m_GraphicsContext = CreateScope<Graphics::GraphicsContext>(*m_Window);
    }

    Application::~Application() {
        LOG_INFO("Shutting down Application");
        m_GraphicsContext.release();
        m_Window.release();
    }

    void Application::Run() {
        LOG_INFO("Running Application");
        while(!m_Window->ShouldClose()){
            m_Window->Update();

            m_GraphicsContext->GetDevice().Tick();
        }
    }
}