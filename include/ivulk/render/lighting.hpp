/**
 * @file lighting.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Utilities and types for lighting.
 */

#pragma once

#include <ivulk/glm.hpp>

namespace ivulk {
    struct PointLightAttenuation
    {
        LAYOUT_FLOAT float constant;
        LAYOUT_FLOAT float linear;
        LAYOUT_FLOAT float quadratic;

        static PointLightAttenuation fromRadius(const float radius)
        {
            const float constant  = 1.0f;
            const float linear    = 2.0f / radius;
            const float quadratic = 1.0f / (radius * radius);

            return {
                .constant  = constant,
                .linear    = linear,
                .quadratic = quadratic,
            };
        }
    };

    struct PointLight
    {
        LAYOUT_VEC3 glm::vec3 position;
        LAYOUT_VEC3 glm::vec3 color;

        LAYOUT_STRUCT PointLightAttenuation attenuation;
    };

    struct DirLight
    {
        LAYOUT_VEC3 glm::vec3 direction;
        LAYOUT_VEC3 glm::vec3 color;
    };
} // namespace ivulk
