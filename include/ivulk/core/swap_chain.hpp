/**
 * @file swap_chain.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Vulkan swapchain support.
 */

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ivulk {
    /**
	 * @brief Information about the swap chain's support for Vulkan features
	 */
    struct SwapChainInfo
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

} // namespace ivulk
