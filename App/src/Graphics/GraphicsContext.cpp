#include "GraphicsContext.h"

#include "Core/Logger.h"
#include <iostream>

namespace Minecraft::Graphics
{
    GraphicsContext::GraphicsContext(Window &window)
        : m_Window(window)
    {
        // Create Instance
        static constexpr auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        wgpu::InstanceDescriptor instanceDescriptor{
            .requiredFeatureCount = 1,
            .requiredFeatures = &kTimedWaitAny};

        LOG_INFO("Creating wgpu Instance");
        wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
        if (instance == nullptr)
        {
            LOG_ERROR("Instance Creation failed!");
            return;
        }

        // Create Surface
        m_Surface = m_Window.CreateSurface(instance);

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
        instance.WaitAny(instance.RequestAdapter(&options, callbackMode, callback, userdata), UINT64_MAX);
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
            });

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
        surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
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
        depthTextureDesc.label = "Depth Texture";
        depthTextureDesc.size.width = m_Window.GetWidth();
        depthTextureDesc.size.height = m_Window.GetHeight();
        depthTextureDesc.size.depthOrArrayLayers = 1;
        depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
        depthTextureDesc.format = m_DepthFormat;
        wgpu::Texture depthTexture = m_Device.CreateTexture(&depthTextureDesc);

        m_DepthTextureView = depthTexture.CreateView();
        depthTexture = nullptr;

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

    std::pair<wgpu::SurfaceTexture, wgpu::TextureView> GraphicsContext::AcquireNextTexture()
    {
        // Get surface texture
        wgpu::SurfaceTexture surfaceTexture{};
        m_Surface.GetCurrentTexture(&surfaceTexture);

        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Outdated || surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Lost)
        {
            // resize surface
            wgpu::SurfaceConfiguration surfaceConfig{};
            surfaceConfig.nextInChain = nullptr;
            surfaceConfig.device = m_Device;
            surfaceConfig.width = m_Window.GetWidth();
            surfaceConfig.height = m_Window.GetHeight();
            surfaceConfig.viewFormatCount = 0;
            surfaceConfig.viewFormats = nullptr;
            surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
            surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
            surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;

            wgpu::SurfaceCapabilities surfaceCapabilities{};
            m_Surface.GetCapabilities(m_Adapter, &surfaceCapabilities);
            surfaceConfig.format = surfaceCapabilities.formats[0];
            m_Surface.Configure(&surfaceConfig);

            m_DepthTextureView = nullptr;

            wgpu::TextureDescriptor depthTextureDesc{};
            depthTextureDesc.nextInChain = nullptr;
            depthTextureDesc.label = "Depth Texture";
            depthTextureDesc.size.width = m_Window.GetWidth();
            depthTextureDesc.size.height = m_Window.GetHeight();
            depthTextureDesc.size.depthOrArrayLayers = 1;
            depthTextureDesc.mipLevelCount = 1;
            depthTextureDesc.sampleCount = 1;
            depthTextureDesc.dimension = wgpu::TextureDimension::e2D;
            depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
            depthTextureDesc.format = m_DepthFormat;

            wgpu::Texture depthTexture = m_Device.CreateTexture(&depthTextureDesc);

            if (depthTexture == nullptr)
            {
                LOG_ERROR("Failed to create resized depth texture");
            }

            m_DepthTextureView = depthTexture.CreateView();

            m_Surface.GetCurrentTexture(&surfaceTexture);
        }

        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal && surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal)
        {
            LOG_ERROR("Failed to acquire next texture: {}", static_cast<uint32_t>(surfaceTexture.status));
            return {surfaceTexture, nullptr};
        }

        // Get texture view
        wgpu::TextureViewDescriptor viewDesc{};
        viewDesc.nextInChain = nullptr;
        viewDesc.label = "Surface Texture View";
        viewDesc.format = surfaceTexture.texture.GetFormat();
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = wgpu::TextureAspect::All;

        wgpu::TextureView textureView = surfaceTexture.texture.CreateView(&viewDesc);

        return {surfaceTexture, textureView};
    }
}