/**
 * @file command_buffer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `CommandBuffer` class.
 */

#pragma once
#include <ivulk/core/vulkan_resource.hpp>

#include <ivulk/utils/keywords.hpp>
#include <ivulk/utils/messages.hpp>

#include <glm/glm.hpp>
#include <optional>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace ivulk {
	class Buffer;
	class GraphicsPipeline;
	struct CommandBuffersCreateInfo
	{
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandPool cmdPool = VK_NULL_HANDLE;
		uint32_t count = 1;
	};
	class CommandBuffers : public VulkanResource<CommandBuffers, CommandBuffersCreateInfo, VkCommandPool,
												 std::vector<VkCommandBuffer>>
	{
	public:
		CommandBuffers(VkDevice device, VkCommandPool pool, std::vector<VkCommandBuffer> buffers)
			: base_t(device, handles_t {pool, buffers})
		{ }

		VkCommandPool getCmdPool() { return getHandleAt<0>(); }
		VkCommandBuffer getCmdBuffer(std::size_t i) { return getHandleAt<1>()[i]; }
		std::vector<VkCommandBuffer> getCmdBuffers() { return getHandleAt<1>(); }

		// clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), start, tag, 
			(required
				(index, *)
			)
			(optional
				(flags, *, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)
			)
		)
		// clang-format on
		{
			startImpl(index, flags);
		}

		void finish();

		// clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), draw, tag, 
			(optional
				(vertexBuffer,  *, std::weak_ptr<Buffer>())
				(indexBuffer,   *, std::weak_ptr<Buffer>())
				(vertices,      *,                      0u)
				(instances,     *,                      1u)
				(firstVertex,   *,                      0u)
				(firstInstance, *,                      0u)
			)
		)
		// clang-format on
		{
			drawImpl(vertexBuffer, indexBuffer, vertices, instances, firstVertex, firstInstance);
		}

		// clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), clearAttachments, tag, 
			(required
				(pipeline, *)
			)
			(optional
				(color, *, glm::vec4(0,0,0,1))
			)
		)
		// clang-format on
		{
			clearAttachmentsImpl(pipeline, color);
		}

		// clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), bindPipeline, tag, 
			(required
				(pipeline, *)
			)
		)
		// clang-format on
		{
			bindPipelineImpl(pipeline);
		}

	private:
		friend base_t;

		static CommandBuffers* createImpl(VkDevice device, CommandBuffersCreateInfo createInfo)
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

			return new CommandBuffers(device, createInfo.cmdPool, commandBuffers);
		}

		void destroyImpl() { }

		void startImpl(std::size_t index, VkCommandBufferUsageFlags flags);
		void drawImpl(std::weak_ptr<Buffer> vertexBuffer, std::weak_ptr<Buffer> indexBuffer, uint32_t vertices, uint32_t instances,
					  uint32_t firstVertex, uint32_t firstInstance);
		void clearAttachmentsImpl(std::weak_ptr<GraphicsPipeline> pipeline, glm::vec4 color);

		void bindPipelineImpl(std::weak_ptr<GraphicsPipeline> pipeline);

		std::optional<std::size_t> m_currentIdx = {};
	};
} // namespace ivulk
