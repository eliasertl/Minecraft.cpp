#pragma once

#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>

namespace Minecraft::Graphics
{
    class TerrainRenderable
    {
        public:
            virtual ~TerrainRenderable() = default;

            virtual void Render(wgpu::RenderPassEncoder &renderPass, glm::vec3 cameraPosition) = 0;
    };

    class WireframeRenderable
    {
    public:
        virtual ~WireframeRenderable() = default;

        virtual void RenderWireframe(wgpu::RenderPassEncoder &renderPass, glm::vec3 cameraPosition) = 0;
    };
}