#pragma once

#include "Window.h"
#include <webgpu/webgpu_cpp.h>

namespace Minecraft::Graphics
{
    class ImGuiContext
    {
    public:
        ImGuiContext(Window& window, wgpu::Device device, wgpu::Queue queue, wgpu::TextureFormat surfaceFormat);
        ~ImGuiContext();

        void NewFrame();
        void Render(wgpu::RenderPassEncoder encoder);
    };
}