#include <ivulk/core/app.hpp>
#include <ivulk/utils/messages.hpp>

#include <stdexcept>

namespace ivulk {
	void App::createVkCommandPools()
	{
		QueueFamilyIndices qfIndices = findVkQueueFamilies(state.vk.physicalDevice);

		VkCommandPoolCreateInfo gfxPoolInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = 0,
			.queueFamilyIndex = qfIndices.graphics.value(),
		};

		if (vkCreateCommandPool(state.vk.device, &gfxPoolInfo, nullptr, &state.vk.cmd.gfxPool) != VK_SUCCESS)
		{
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan command pool"));
		}
		else if (getPrintDbg())
		{
			std::cout << utils::makeSuccessMessage("VK::CREATE", "Created Vulkan command pool") << std::endl;
		}
	}

	void App::createVkCommandBuffers()
	{
		auto& cmd = state.vk.cmd;
		auto& fb = state.vk.swapChain.framebuffers;
		cmd.gfxBuffers.resize(fb.size());

		VkCommandBufferAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = cmd.gfxPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(cmd.gfxBuffers.size()),
		};

		if (vkAllocateCommandBuffers(state.vk.device, &allocInfo, cmd.gfxBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error(
				utils::makeErrorMessage("VK::ALLOC", "Failed to allocate Vulkan command buffers"));
		}

		// Configure render passes
		if (auto pipeline = state.vk.pipelines.mainGfx.lock())
		{
			auto& scExtent = state.vk.swapChain.extent;
			VkClearValue clearColor {0.0f, 0.0f, 0.0f, 1.0f};
			for (std::size_t i = 0; i < cmd.gfxBuffers.size(); ++i)
			{
				VkCommandBufferBeginInfo beginInfo {
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.flags = 0,
					.pInheritanceInfo = nullptr,
				};
				if (vkBeginCommandBuffer(cmd.gfxBuffers[i], &beginInfo) != VK_SUCCESS)
				{
					throw std::runtime_error(utils::makeErrorMessage(
						"VK::CMD", "Failed to begin Vulkan command buffer recording"));
				}

				VkRenderPassBeginInfo renderPassInfo {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = pipeline->getRenderPass(),
					.framebuffer = fb[i],
					.renderArea = {
						.offset = {0, 0},
						.extent = scExtent,
					},
					.clearValueCount = 1,
					.pClearValues = &clearColor,
				};
				vkCmdBeginRenderPass(cmd.gfxBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				render(cmd.gfxBuffers[i]);

				vkCmdEndRenderPass(cmd.gfxBuffers[i]);

				if (vkEndCommandBuffer(cmd.gfxBuffers[i]) != VK_SUCCESS)
				{
					throw std::runtime_error(utils::makeErrorMessage(
						"VK::CMD", "Failed to complete Vulkan command buffer recording"));
				}
			}
		}
	}

	void App::drawFrame()
	{
		vkWaitForFences(state.vk.device, 1, &state.vk.sync.inFlightFences[m_currentFrame], VK_TRUE,
						UINT64_MAX);
		uint32_t imageIndex;

		vkAcquireNextImageKHR(state.vk.device, state.vk.swapChain.sc, UINT64_MAX,
							  state.vk.sync.imageAvailableSems[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (state.vk.sync.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(state.vk.device, 1, &state.vk.sync.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		state.vk.sync.imagesInFlight[imageIndex] = state.vk.sync.inFlightFences[m_currentFrame];

		VkSemaphore signalSemaphores[] = {state.vk.sync.renderFinishedSems[m_currentFrame]};
		VkSemaphore waitSemaphores[] = {state.vk.sync.imageAvailableSems[m_currentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &state.vk.cmd.gfxBuffers[imageIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signalSemaphores,
		};

		vkResetFences(state.vk.device, 1, &state.vk.sync.inFlightFences[m_currentFrame]);

		if (vkQueueSubmit(state.vk.queues.graphics, 1, &submitInfo,
						  state.vk.sync.inFlightFences[m_currentFrame])
			!= VK_SUCCESS)
		{
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CMD", "Failed to Vulkan submit draw command buffer"));
		}

		VkSwapchainKHR swapChains[] = {state.vk.swapChain.sc};
		VkPresentInfoKHR presentInfo {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signalSemaphores,
			.swapchainCount = 1,
			.pSwapchains = swapChains,
			.pImageIndices = &imageIndex,
		};

		vkQueuePresentKHR(state.vk.queues.present, &presentInfo);

		m_currentFrame = (m_currentFrame + 1) % m_initArgs.vk.maxFramesInFlight;

	}
} // namespace ivulk
