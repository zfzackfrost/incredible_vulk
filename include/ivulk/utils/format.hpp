/**
 * @file image.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Helper functions for images.
 */

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ivulk::utils {
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
								 VkImageTiling tiling,
								 VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);
} // namespace ivulk::utils
