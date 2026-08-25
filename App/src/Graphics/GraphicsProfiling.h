#pragma once

#include <tracy/Tracy.hpp>
#include <tracy/TracyWebGPU.hpp>

namespace Minecraft::Graphics
{
    class GraphicsProfiling
    {
    public:
        static tracy::WebGPUQueueCtx* GetTracyQueueContext();
    };
}

#define TracyGPUZoneN(encoder, passDesc, name) TracyWebGPUZone(Minecraft::Graphics::GraphicsProfiling::GetTracyQueueContext(), encoder.Get(), passDesc, name)