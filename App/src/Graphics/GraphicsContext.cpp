#include "GraphicsContext.h"
#include "GraphicsProfiling.h"

#include "Core/Logger.h"
#include <iostream>

#include <tracy/Tracy.hpp>
#include <tracy/TracyWebGPU.hpp>

namespace Minecraft::Graphics
{
    tracy::WebGPUQueueCtx *g_TracyCTX = nullptr;
    wgpu::BackendType g_Backend = wgpu::BackendType::Undefined;
    wgpu::PresentMode g_PresentMode = wgpu::PresentMode::Immediate;

    tracy::WebGPUQueueCtx *GraphicsProfiling::GetTracyQueueContext()
    {
        return g_TracyCTX;
    }

    GraphicsContext::GraphicsContext(Window &window)
        : m_Window(window)
    {
        // Create Instance
        static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        wgpu::InstanceDescriptor instanceDescriptor{
            .requiredFeatureCount = 1,
            .requiredFeatures = &kTimedWaitAny};

        LOG_INFO("Creating wgpu Instance");
        m_Instance = wgpu::CreateInstance(&instanceDescriptor);
        if (m_Instance == nullptr)
        {
            LOG_ERROR("Instance Creation failed!");
            return;
        }

        // Create Surface
        m_Surface = m_Window.CreateSurface(m_Instance);

        // Create Adapter
        wgpu::RequestAdapterOptions options = {};
        options.compatibleSurface = m_Surface;

        auto callback = [](wgpu::RequestAdapterStatus status, wgpu::Adapter m_Adapter, wgpu::StringView message, void *userdata)
        {
            if (status != wgpu::RequestAdapterStatus::Success)
            {
                LOG_ERROR("Failed to get an adapter: {}", message);
                return;
            }
            *static_cast<wgpu::Adapter *>(userdata) = m_Adapter;
        };

        auto callbackMode = wgpu::CallbackMode::WaitAnyOnly;
        void *userdata = &m_Adapter;
        m_Instance.WaitAny(m_Instance.RequestAdapter(&options, callbackMode, callback, userdata), UINT64_MAX);
        if (m_Adapter == nullptr)
        {
            LOG_ERROR("RequestAdapter failed!\n");
            return;
        }

        wgpu::DawnAdapterPropertiesPowerPreference power_props{};

        wgpu::AdapterInfo info{};
        info.nextInChain = &power_props;

        m_Adapter.GetInfo(&info);
        LOG_INFO("VendorID: {:#x}", info.vendorID);
        LOG_INFO("Vendor: {}", info.vendor);
        LOG_INFO("Architecture: {}", info.architecture);
        LOG_INFO("DeviceID: {:#x}", info.deviceID);
        LOG_INFO("Name: {}", info.device);
        LOG_INFO("Driver description: {}", info.description);
        switch (info.backendType)
        {
        case wgpu::BackendType::Null:
            LOG_INFO("Backend: Null");
            break;
        case wgpu::BackendType::D3D11:
            LOG_INFO("Backend: D3D11");
            break;
        case wgpu::BackendType::D3D12:
            LOG_INFO("Backend: D3D12");
            break;
        case wgpu::BackendType::Metal:
            LOG_INFO("Backend: Metal");
            break;
        case wgpu::BackendType::Vulkan:
            LOG_INFO("Backend: Vulkan");
            break;
        case wgpu::BackendType::OpenGL:
            LOG_INFO("Backend: OpenGL");
            break;
        case wgpu::BackendType::OpenGLES:
            LOG_INFO("Backend: OpenGLES");
            break;
        default:
            LOG_ERROR("Unknown backend type!");
        }

        wgpu::Limits limits{};
        m_Adapter.GetLimits(&limits);

        // Create Device
        auto deviceDescriptor = wgpu::DeviceDescriptor{};
        deviceDescriptor.requiredLimits = &limits;
        deviceDescriptor.defaultQueue.label = "Default Queue";
        deviceDescriptor.SetUncapturedErrorCallback(
            [](wgpu::Device const &device,
               wgpu::ErrorType type,
               wgpu::StringView message)
            {
                LOG_ERROR(
                    "Dawn uncaptured error [{}]: {}",
                    static_cast<uint32_t>(type),
                    message);
                __debugbreak();
            });
        TracyWebGPUSetupDeviceDescriptor(reinterpret_cast<WGPUDeviceDescriptor &>(deviceDescriptor));

        LOG_INFO("Creating wgpu Device");
        m_Device = m_Adapter.CreateDevice(&deviceDescriptor);
        if (m_Device == nullptr)
        {
            LOG_ERROR("Device Creation failed!");
            return;
        }

        // Get Queue
        m_Queue = m_Device.GetQueue();
        auto onWorkDone = [](wgpu::QueueWorkDoneStatus status, wgpu::StringView message)
        {
            if (status != wgpu::QueueWorkDoneStatus::Success)
            {
                LOG_ERROR("Queue work done callback failed: {}", message);
                return;
            }
            LOG_INFO("Queue work done callback succeeded!");
        };
        m_Queue.OnSubmittedWorkDone(wgpu::CallbackMode::WaitAnyOnly, onWorkDone);

        // Create Tracy Context
        g_TracyCTX = TracyWebGPUContext(m_Instance.Get(), m_Device.Get(), m_Queue.Get());

        // Configure Surface
        LOG_INFO("Configuring wgpu Surface");
        wgpu::SurfaceConfiguration surfaceConfig{};
        surfaceConfig.nextInChain = nullptr;
        surfaceConfig.device = m_Device;
        surfaceConfig.width = m_Window.GetWidth();
        surfaceConfig.height = m_Window.GetHeight();
        surfaceConfig.viewFormatCount = 0;
        surfaceConfig.viewFormats = nullptr;
        surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
        surfaceConfig.presentMode = g_PresentMode;
        surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;

        wgpu::SurfaceCapabilities surfaceCapabilities{};
        m_Surface.GetCapabilities(m_Adapter, &surfaceCapabilities);
        surfaceConfig.format = surfaceCapabilities.formats[0];
        m_SurfaceFormat = surfaceCapabilities.formats[0];

        m_Surface.Configure(&surfaceConfig);

        // Create Depth Texture
        LOG_INFO("Creating Depth Texture");
        m_DepthFormat = wgpu::TextureFormat::Depth24Plus;

        wgpu::TextureDescriptor depthTextureDesc{};
        depthTextureDesc.nextInChain = nullptr;
        depthTextureDesc.label = "[GraphicsContext] Depth Texture";
        depthTextureDesc.size.width = m_Window.GetWidth();
        depthTextureDesc.size.height = m_Window.GetHeight();
        depthTextureDesc.size.depthOrArrayLayers = 1;
        depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
        depthTextureDesc.format = m_DepthFormat;
        wgpu::Texture depthTexture = m_Device.CreateTexture(&depthTextureDesc);

        m_DepthTextureView = depthTexture.CreateView();
        depthTexture = nullptr;

        m_SurfaceWidth = m_Window.GetWidth();
        m_SurfaceHeight = m_Window.GetHeight();

        return;
    }

    GraphicsContext::~GraphicsContext()
    {
        LOG_INFO("Destroying GraphicsContext");
        m_DepthTextureView = nullptr;
        m_Queue = nullptr;
        m_Device = nullptr;
        m_Adapter = nullptr;
        m_Instance = nullptr;
        m_Surface.Unconfigure();
        m_Surface = nullptr;
    }

    void GraphicsContext::FrameEnd()
    {
        ZoneScoped;
        g_TracyCTX->Collect(true);
    }

    std::pair<wgpu::SurfaceTexture, wgpu::TextureView>
    GraphicsContext::AcquireNextTexture()
    {
        ZoneScoped;

        wgpu::SurfaceTexture surfaceTexture{};

        {
            ZoneScopedN("GetCurrentTexture");
            m_Surface.GetCurrentTexture(&surfaceTexture);
        }

        auto width = m_Window.GetWidth();
        auto height = m_Window.GetHeight();

        if (m_SurfaceWidth != width || m_SurfaceHeight != height)
        {
            ZoneScopedN("Resize Surface");

            {
                ZoneScopedN("Surface Configure");

                wgpu::SurfaceConfiguration surfaceConfig{};
                surfaceConfig.device = m_Device;
                surfaceConfig.width = width;
                surfaceConfig.height = height;
                surfaceConfig.format = m_SurfaceFormat;
                surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
                surfaceConfig.presentMode = g_PresentMode;
                surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;

                m_Surface.Configure(&surfaceConfig);
            }

            {
                ZoneScopedN("Create Depth Texture");

                m_DepthTextureView = nullptr;

                wgpu::TextureDescriptor depthTextureDesc{};
                depthTextureDesc.label = "[GraphicsContext] Depth Texture";
                depthTextureDesc.size = {width, height, 1};
                depthTextureDesc.mipLevelCount = 1;
                depthTextureDesc.sampleCount = 1;
                depthTextureDesc.dimension = wgpu::TextureDimension::e2D;
                depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
                depthTextureDesc.format = m_DepthFormat;

                wgpu::Texture depthTexture =
                    m_Device.CreateTexture(&depthTextureDesc);

                m_DepthTextureView = depthTexture.CreateView();
            }

            {
                ZoneScopedN("GetCurrentTexture After Resize");
                m_Surface.GetCurrentTexture(&surfaceTexture);
            }

            m_SurfaceWidth = width;
            m_SurfaceHeight = height;
        }

        if (surfaceTexture.status !=
                wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
            surfaceTexture.status !=
                wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
        {
            LOG_ERROR(
                "Failed to acquire next texture: {}",
                static_cast<uint32_t>(surfaceTexture.status));

            return {surfaceTexture, nullptr};
        }

        wgpu::TextureView textureView;

        {
            ZoneScopedN("Create Surface Texture View");

            wgpu::TextureViewDescriptor viewDesc{};
            viewDesc.label = "[GraphicsContext] Surface Texture View";
            viewDesc.format = surfaceTexture.texture.GetFormat();
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseMipLevel = 0;
            viewDesc.mipLevelCount = 1;
            viewDesc.baseArrayLayer = 0;
            viewDesc.arrayLayerCount = 1;
            viewDesc.aspect = wgpu::TextureAspect::All;

            textureView = surfaceTexture.texture.CreateView(&viewDesc);
        }

        return {surfaceTexture, textureView};
    }
}