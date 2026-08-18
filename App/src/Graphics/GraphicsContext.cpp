#include "GraphicsContext.h"

#include "Core/Logger.h"
#include <iostream>

namespace Minecraft::Graphics
{
    GraphicsContext::GraphicsContext()
    {
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
        // Synchronously request the adapter.
        wgpu::RequestAdapterOptions options = {};
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
        return;
    }

    GraphicsContext::~GraphicsContext()
    {
    }

    wgpu::Instance GraphicsContext::GetInstance() const
    {
        return m_Instance;
    }

    wgpu::Adapter GraphicsContext::GetAdapter() const
    {
        return m_Adapter;
    }
}