/**
 * @file priorities.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Constants for rendering order priorities.
 */

#pragma once

#include <ivulk/config.hpp>

#include <cstdint>

namespace ivulk {
    namespace E_RenderPriority {
        constexpr int16_t Overlay     = 3000;
        constexpr int16_t Normal      = 2000;
        constexpr int16_t Transparent = 1000;
        constexpr int16_t Background  = 0000;
    } // namespace E_RenderPriority
} // namespace ivulk
