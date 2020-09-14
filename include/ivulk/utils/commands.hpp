/**
 * @file commands.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Helper functions for low-level comand buffer usage.
 */
#pragma once

#include <ivulk/config.hpp>

#include <ivulk/vk.hpp>

#include <ivulk/core/app.hpp>

namespace ivulk::utils {
	inline VkCommandBuffer beginOneTimeCommands()
	{
		auto state = App::current()->getState().vk;

		VkCommandBufferAllocateInfo allocInfo {
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool        = state.cmd.gfxPool,
			.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(state.device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	inline void endOneTimeCommands(const VkCommandBuffer commandBuffer)
	{
		auto state = App::current()->getState().vk;
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo {
			.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers    = &commandBuffer,
		};

		vkQueueSubmit(state.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(state.queues.graphics);

		vkFreeCommandBuffers(state.device, state.cmd.gfxPool, 1, &commandBuffer);
	}

	inline void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginOneTimeCommands();

		VkBufferImageCopy region {
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = {0, 0, 0},
			.imageExtent = {width, height, 1},
		};


		// clang-format off
		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
		// clang-format on

		endOneTimeCommands(commandBuffer);
	}
} // namespace ivulk::utils
