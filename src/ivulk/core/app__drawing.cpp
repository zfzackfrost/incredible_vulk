#include <ivulk/core/app.hpp>
#include <ivulk/utils/messages.hpp>

#include <stdexcept>

namespace ivulk {
    void App::createVkCommandPools()
    {
        QueueFamilyIndices qfIndices = findVkQueueFamilies(state.vk.physicalDevice);

        VkCommandPoolCreateInfo gfxPoolInfo {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
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

    void App::createVkCommandBuffers(std::size_t imageIndex)
    {
        auto& fb = state.vk.swapChain.framebuffers;
        if (!state.vk.cmd.renderCmdBufs)
        {
            state.vk.cmd.renderCmdBufs = CommandBuffers::create(state.vk.device,
                                                                {
                                                                    .cmdPool = state.vk.cmd.gfxPool,
                                                                });
        }
        auto cmdBufs = state.vk.cmd.renderCmdBufs;
        // Configure render passes
        if (auto pipeline = state.vk.pipelines.mainGfx.lock())
        {
            auto& scExtent = state.vk.swapChain.extent;
            VkClearValue clearColor {0.0f, 0.0f, 0.0f, 1.0f};
            std::array<VkClearValue, 2> clearValues {};
            clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};

            {
                cmdBufs->start(0);

                VkRenderPassBeginInfo renderPassInfo {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = pipeline->getRenderPass(),
					.framebuffer = fb[imageIndex],
					.renderArea = {
						.offset = {0, 0},
						.extent = scExtent,
					},
					.clearValueCount = static_cast<uint32_t>(clearValues.size()),
					.pClearValues = clearValues.data(),
				};
                vkCmdBeginRenderPass(cmdBufs->getCmdBuffer(0), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                render(cmdBufs);

                vkCmdEndRenderPass(cmdBufs->getCmdBuffer(0));

                cmdBufs->finish();
            }
        }
    }

    void App::drawFrame()
    {
        vkQueueWaitIdle(state.vk.queues.graphics);

        m_currentFrame = 0;
        uint32_t imageIndex;

        auto result = vkAcquireNextImageKHR(state.vk.device,
                                            state.vk.swapChain.sc,
                                            UINT64_MAX,
                                            state.vk.sync.imageAvailableSems[m_currentFrame],
                                            VK_NULL_HANDLE,
                                            &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateVkSwapChain();
            return;
        }

        if (state.vk.sync.imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(
                state.vk.device, 1, &state.vk.sync.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }

        createVkCommandBuffers(imageIndex);
        auto cmdBufs = state.vk.cmd.renderCmdBufs;
        auto cmdBuf0 = cmdBufs->getCmdBuffer(0);

        state.vk.sync.imagesInFlight[imageIndex] = state.vk.sync.inFlightFences[m_currentFrame];

        VkSemaphore signalSemaphores[]    = {state.vk.sync.renderFinishedSems[m_currentFrame]};
        VkSemaphore waitSemaphores[]      = {state.vk.sync.imageAvailableSems[m_currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo {
            .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = waitSemaphores,
            .pWaitDstStageMask    = waitStages,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &cmdBuf0,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = signalSemaphores,
        };

        vkResetFences(state.vk.device, 1, &state.vk.sync.inFlightFences[m_currentFrame]);
        if (vkQueueSubmit(
                state.vk.queues.graphics, 1, &submitInfo, state.vk.sync.inFlightFences[m_currentFrame])
            != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Failed to submit Vulkan draw command buffer"));
        }

        VkSwapchainKHR swapChains[] = {state.vk.swapChain.sc};
        VkPresentInfoKHR presentInfo {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = signalSemaphores,
            .swapchainCount     = 1,
            .pSwapchains        = swapChains,
            .pImageIndices      = &imageIndex,
        };

        vkQueuePresentKHR(state.vk.queues.present, &presentInfo);

        m_currentFrame = (m_currentFrame + 1) % m_initArgs.vk.maxFramesInFlight;

        vkQueueWaitIdle(state.vk.queues.present);
    }
} // namespace ivulk
