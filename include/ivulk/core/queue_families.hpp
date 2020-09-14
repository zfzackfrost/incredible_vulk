/**
 * @file queue_family.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `QueueFamilies` structure.
 */

#pragma once

#include <ivulk/config.hpp>

#include <cstdint>
#include <optional>
#include <set>

namespace ivulk {
    /**
	 * @brief Indices of Vulkan queue families
	 */
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics = {}; ///< Index of the graphics queue family
        std::optional<uint32_t> present  = {}; ///< Index of the present queue family

        bool isComplete() const { return graphics.has_value() && present.has_value(); }

        std::set<uint32_t> getUniqueFamilies() const
        {
            return {
                graphics.value(),
                present.value(),
            };
        }
    };
} // namespace ivulk
