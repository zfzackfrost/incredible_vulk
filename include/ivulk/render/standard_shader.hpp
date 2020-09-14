/**
 * @file standard_shader.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Helpers for the standard shader setup.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/glm.hpp>

namespace ivulk {
    struct MatricesPushConstants
    {
        glm::mat4 model;
    };
    struct MatricesUBO
    {
        LAYOUT_MAT4 glm::mat4 view;
        LAYOUT_MAT4 glm::mat4 proj;
    };
} // namespace ivulk
