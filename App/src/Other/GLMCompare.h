#pragma once

#include <glm/glm.hpp>

template<typename T>
struct GenericVectorLess
{
    bool operator()(const T& a, const T& b) const noexcept
    {
        for (int i = 0; i < T::length(); i++)
        {
            if (a[i] != b[i]) return a[i] < b[i];
        }
        return false;
    }
};

using IVec2Less = GenericVectorLess<glm::ivec2>;
using IVec3Less = GenericVectorLess<glm::ivec3>;
using IVec4Less = GenericVectorLess<glm::ivec4>;

using Vec2Less = GenericVectorLess<glm::vec2>;
using Vec3Less = GenericVectorLess<glm::vec3>;
using Vec4Less = GenericVectorLess<glm::vec4>;