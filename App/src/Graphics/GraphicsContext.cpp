#include "GraphicsContext.h"

#include "Core/Logger.h"
#include <iostream>

namespace Minecraft::Graphics
{
    GraphicsContext::GraphicsContext(Window& window)
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
        m_Surface = window.CreateSurface(instance);

        // Create Adapter
        wgpu::RequestAdapterOptions options = {};
        options.compatibleSurface = m_Surface;
        wgpu::Adapter adapter;

        auto callback = [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message, void *userdata)
        {
            if (status != wgpu::RequestAdapterStatus::Success)
            {
                LOG_ERROR("Failed to get an adapter: {}", message);
                return;
            }
            *static_cast<wgpu::Adapter *>(userdata) = adapter;
        };

        auto callbackMode = wgpu::CallbackMode::WaitAnyOnly;
        void *userdata = &adapter;
        instance.WaitAny(instance.RequestAdapter(&options, callbackMode, callback, userdata), UINT64_MAX);
        if (adapter == nullptr)
        {
            LOG_ERROR("RequestAdapter failed!\n");
            return;
        }

        wgpu::DawnAdapterPropertiesPowerPreference power_props{};

        wgpu::AdapterInfo info{};
        info.nextInChain = &power_props;

        adapter.GetInfo(&info);
        LOG_INFO("VendorID: {:#x}", info.vendorID);
        LOG_INFO("Vendor: {}", info.vendor);
        LOG_INFO("Architecture: {}", info.architecture);
        LOG_INFO("DeviceID: {:#x}", info.deviceID);
        LOG_INFO("Name: {}", info.device);
        LOG_INFO("Driver description: {}", info.description);

        wgpu::Limits limits{};
        adapter.GetLimits(&limits);

        // Create Device
        auto deviceDescriptor = wgpu::DeviceDescriptor{};
        deviceDescriptor.requiredLimits = &limits;
        deviceDescriptor.defaultQueue.label = "Default Queue";

        LOG_INFO("Creating wgpu Device");
        wgpu::Device device = adapter.CreateDevice(&deviceDescriptor);
        if (device == nullptr)
        {
            LOG_ERROR("Device Creation failed!");
            return;
        }
        m_Device = device;
        
        // Get Queue
        m_Queue = device.GetQueue();
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
        return;
    }

    GraphicsContext::~GraphicsContext()
    {
        LOG_INFO("Destroying GraphicsContext");
        m_Queue = nullptr;
        m_Device = nullptr;
        m_Adapter = nullptr;
        m_Instance = nullptr;
        m_Surface.Unconfigure();
        m_Surface = nullptr;    
    }
}