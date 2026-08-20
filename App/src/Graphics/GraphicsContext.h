#pragma once

#include "Window.h"
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>
#include <utility>

namespace Minecraft::Graphics
{
    class GraphicsContext
    {
    public:
        GraphicsContext(Window& window);
        ~GraphicsContext();
        
        std::pair<wgpu::SurfaceTexture, wgpu::TextureView> AcquireNextTexture();

        inline wgpu::Instance GetInstance() const { return m_Instance; }
        inline wgpu::Surface GetSurface() const { return m_Surface; }
        inline wgpu::TextureFormat GetSurfaceFormat() const { return m_SurfaceFormat; }
        inline wgpu::TextureView GetDepthTextureView() const { return m_DepthTextureView; }
        inline wgpu::TextureFormat GetDepthFormat() const { return m_DepthFormat; }
        inline wgpu::Adapter GetAdapter() const { return m_Adapter; }
        inline wgpu::Device GetDevice() const { return m_Device; }
        inline wgpu::Queue GetQueue() const { return m_Queue; }

    private:
        Window& m_Window;
        wgpu::Instance m_Instance;
        wgpu::Surface m_Surface;
        wgpu::TextureFormat m_SurfaceFormat;
        wgpu::TextureView m_DepthTextureView;
        wgpu::TextureFormat m_DepthFormat;
        wgpu::Adapter m_Adapter;
        wgpu::Device m_Device;
        wgpu::Queue m_Queue;
    };
}