/**
 * @file scene_ubo.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Types for UBOs holding scene data
 */

#pragma once

#include <ivulk/glm.hpp>
#include <ivulk/render/lighting.hpp>

namespace ivulk {
    constexpr std::size_t POINT_LIGHTS_PER_PASS = 4;
    constexpr std::size_t DIR_LIGHTS_PER_PASS   = 4;

    struct SceneUBOData
    {
        LAYOUT_STRUCT PointLight pointLights[POINT_LIGHTS_PER_PASS];
        LAYOUT_STRUCT DirLight dirLights[DIR_LIGHTS_PER_PASS];

        LAYOUT_INT int32_t pointLightCount = 0;
        LAYOUT_INT int32_t dirLightCount   = 0;

        LAYOUT_VEC3 glm::vec3 viewPosition;
    };
} // namespace ivulk
