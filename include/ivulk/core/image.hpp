/**
 * @file image.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Image` class.
 */

#pragma once

#include <vulkan/vulkan.h>

#include <ivulk/core/vma.hpp>
#include <ivulk/core/vulkan_resource.hpp>

#include <boost/filesystem.hpp>

#include <optional>

namespace ivulk {
	struct ImageInfo final
	{
		struct {
			bool bEnable = false;
			boost::filesystem::path path;
			bool bSrgb = true;
		} load;

		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VmaMemoryUsage memoryMode = E_MemoryMode::GpuOnly;
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		VkExtent3D extent{};
		VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	};

	class Image : public VulkanResource<Image, ImageInfo, VkImage, VmaAllocation, VkImageView>
	{
	public:
		VkImage getImage() { return getHandleAt<0>(); }
		VmaAllocation getAllocation() { return getHandleAt<1>(); }
		VkImageView getImageView() { return getHandleAt<2>(); }

	private:
		friend base_t;

		Image(VkDevice device, VkImage image, VmaAllocation allocation, VkImageView view);

		static Image* createImpl(VkDevice device, ImageInfo createInfo);

		void destroyImpl();
	};
} // namespace ivulk
