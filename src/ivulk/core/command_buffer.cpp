#include <ivulk/core/buffer.hpp>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>

#include <ivulk/utils/messages.hpp>

namespace ivulk {
	void CommandBuffers::startImpl(std::size_t index, VkCommandBufferUsageFlags flags)
	{
		if (m_currentIdx.has_value())
			throw std::runtime_error(utils::makeErrorMessage("VK::CMD", "Command buffer recording already started"));

		VkCommandBufferBeginInfo beginInfo {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = flags;
		beginInfo.pInheritanceInfo = nullptr;
		if (vkBeginCommandBuffer(getCmdBuffer(index), &beginInfo) != VK_SUCCESS)
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CMD", "Failed to start command buffer recording"));
		m_currentIdx = index;
	}
	void CommandBuffers::finish()
	{
		if (!m_currentIdx.has_value())
			throw std::runtime_error(utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));
		m_currentIdx = {};
		if (vkEndCommandBuffer(getCmdBuffer(*m_currentIdx)) != VK_SUCCESS)
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CMD", "Failed to finish command buffer recording"));
	}

	void CommandBuffers::drawImpl(std::weak_ptr<Buffer> buffer, uint32_t vertices, uint32_t instances,
								  uint32_t firstVertex, uint32_t firstInstance)
	{
		if (!m_currentIdx.has_value())
			throw std::runtime_error(utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

		auto vertCount = vertices;
		if (auto buf = buffer.lock())
		{
			VkBuffer buffers[] = {buf->getBuffer()};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(getCmdBuffer(*m_currentIdx), 0, 1, buffers, offsets);
			if (vertCount == 0)
				vertCount = buf->getCount();
		}
		vkCmdDraw(getCmdBuffer(*m_currentIdx), vertCount, instances, firstVertex, firstInstance);
	}

	void CommandBuffers::clearAttachmentsImpl(std::weak_ptr<GraphicsPipeline> pipeline, glm::vec4 color)
	{
		if (!m_currentIdx.has_value())
			throw std::runtime_error(utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

		if (auto pl = pipeline.lock())
		{
			auto colorIndices = pl->getColorAttIndices();
			std::vector<VkClearAttachment> clearAtts;
			for (auto& ci : colorIndices)
			{
				clearAtts.push_back({
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.colorAttachment = ci,
					.clearValue = {
						.color = {
							.float32 = {color.r, color.g, color.b, color.a}
						}
					}
				});
			}
			VkClearRect	rect {
				.rect = {
					.offset = {0, 0},
					.extent = App::current()->getState().vk.swapChain.extent
				},
				.baseArrayLayer = 0,
				.layerCount = 1,
			};
			vkCmdClearAttachments(getCmdBuffer(*m_currentIdx), clearAtts.size(), clearAtts.data(), 1, &rect);

		}
	}

	void CommandBuffers::bindPipelineImpl(std::weak_ptr<GraphicsPipeline> pipeline)
	{
		if (!m_currentIdx.has_value())
			throw std::runtime_error(utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

		if (auto pl = pipeline.lock())
		{
			vkCmdBindPipeline(getCmdBuffer(*m_currentIdx), VK_PIPELINE_BIND_POINT_GRAPHICS, pl->getPipeline());
		}
	}

} // namespace ivulk
