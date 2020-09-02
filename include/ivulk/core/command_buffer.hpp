/**
 * @file command_buffer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `CommandBuffer` class.
 */

#pragma once
#include <ivulk/core/vulkan_resource.hpp>

#include <ivulk/utils/messages.hpp>

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace ivulk {
	struct CommandBufferCreateInfo
	{
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandPool cmdPool = VK_NULL_HANDLE;
		uint32_t count = 1;
	};
	class CommandBuffer : public VulkanResource<CommandBuffer, CommandBufferCreateInfo, VkCommandPool,
												std::vector<VkCommandBuffer>>
	{
	public:
		CommandBuffer(VkDevice device, VkCommandPool pool, std::vector<VkCommandBuffer> buffers)
			: base_t(device, handles_t {pool, buffers})
		{ }

		VkCommandPool getCmdPool() { return getHandleAt<0>(); }
		VkCommandBuffer getCmdBuffer(std::size_t i) { return getHandleAt<1>()[i]; }
		std::vector<VkCommandBuffer> getCmdBuffers() { return getHandleAt<1>(); }

		static CommandBuffer* createImpl(VkDevice device, CommandBufferCreateInfo createInfo)
		{
			VkCommandBufferAllocateInfo allocInfo {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = createInfo.cmdPool,
				.level = createInfo.level,
				.commandBufferCount = createInfo.count,
			};

			std::vector<VkCommandBuffer> commandBuffers(createInfo.count);
			if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			{
				throw std::runtime_error(
					utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan command buffer(s)"));
			}

			return new CommandBuffer(device, createInfo.cmdPool, commandBuffers);
		}

		void destroyImpl() {
		}
	};
} // namespace ivulk
