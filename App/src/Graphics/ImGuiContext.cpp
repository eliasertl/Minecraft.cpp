#include "ImGuiContext.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>

namespace Minecraft::Graphics
{
    ImGuiContext::ImGuiContext(Window& window, wgpu::Device device, wgpu::Queue queue, wgpu::TextureFormat surfaceFormat)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOther(window.GetNativeWindow(), true);

        ImGui_ImplWGPU_InitInfo init_info = {};
        init_info.Device = device.Get();
        init_info.NumFramesInFlight = 3;
        init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(surfaceFormat);
        init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
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
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
    }

    void ImGuiContext::Render(wgpu::RenderPassEncoder encoder)
    {
        ImGui::Render();
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), encoder.Get());
    }
}