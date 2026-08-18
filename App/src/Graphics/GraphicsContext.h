#pragma once

#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

namespace Minecraft::Graphics
{
    class GraphicsContext
    {
    public:
        GraphicsContext();
        ~GraphicsContext();

        wgpu::Instance GetInstance() const;
        wgpu::Adapter GetAdapter() const;

    private:
        wgpu::Instance m_Instance;
        wgpu::Adapter m_Adapter;
    };
}