/**
 * @file shader_stage.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `E_ShaderStage` enum.
 */

#pragma once

#include <ivulk/config.hpp>

#include <vulkan/vulkan.h>

namespace ivulk {
    namespace E_ShaderStage {
        constexpr VkShaderStageFlags Vertex      = VK_SHADER_STAGE_VERTEX_BIT;
        constexpr VkShaderStageFlags TessControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        constexpr VkShaderStageFlags TessEval    = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        constexpr VkShaderStageFlags Geometry    = VK_SHADER_STAGE_GEOMETRY_BIT;
        constexpr VkShaderStageFlags Fragment    = VK_SHADER_STAGE_FRAGMENT_BIT;
        constexpr VkShaderStageFlags Compute     = VK_SHADER_STAGE_COMPUTE_BIT;
        constexpr VkShaderStageFlags AllGraphics = VK_SHADER_STAGE_ALL_GRAPHICS;
        constexpr VkShaderStageFlags All         = VK_SHADER_STAGE_ALL;
    } // namespace E_ShaderStage
} // namespace ivulk
