#include "ImGuiContext.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>

namespace Minecraft::Graphics
{
    ImGuiContext::ImGuiContext(Window &window, wgpu::Device device, wgpu::Queue queue, wgpu::TextureFormat surfaceFormat, wgpu::TextureFormat depthStencilFormat)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();
        ImGuiStyle &style = ImGui::GetStyle();
        float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.2f);
        colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        ImGui_ImplGlfw_InitForOther(window.GetNativeWindow(), true);

        ImGui_ImplWGPU_InitInfo init_info = {};
        init_info.Device = device.Get();
        init_info.NumFramesInFlight = 3;
        init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(surfaceFormat);
        init_info.DepthStencilFormat = static_cast<WGPUTextureFormat>(depthStencilFormat);
        ImGui_ImplWGPU_Init(&init_info);
    }

    ImGuiContext::~ImGuiContext()
    {
        ImGui_ImplWGPU_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiContext::NewFrame()
    {
        ZoneScoped;
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiContext::Render(wgpu::RenderPassEncoder encoder)
    {
        ZoneScoped;
        ImGui::Render();
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), encoder.Get());
    }
}