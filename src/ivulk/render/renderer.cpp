#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/render/renderer.hpp>

#include <ivulk/core/app.hpp>

#include <ivulk/vk.hpp>

namespace ivulk {
    Renderer::Renderer(App* ownerApp)
        : state(ownerApp->state)
        , ownerApp(ownerApp)
    {
        m_currentFrame = 0;
    }

    std::weak_ptr<Renderer> s_current = {};

    std::weak_ptr<Renderer> Renderer::current()
    {
        return s_current;
    }

    void Renderer::activate() { makeCurrent(this); }

    void Renderer::makeCurrent(Renderer* newCurrent)
    {
        if (newCurrent)
            s_current = newCurrent->weak_from_this();
        else
            s_current = {};
    }

    void Renderer::drawFrame()
    {
        ownerApp->state.vk.queues.graphics.waitIdle();
        vkWaitForFences(
            state.vk.device, 1, &state.vk.sync.inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;

        auto result_acquire = state.vk.device.acquireNextImageKHR(
            state.vk.swapChain.sc, UINT64_MAX, state.vk.sync.imageAvailableSems[m_currentFrame], nullptr);

        if (result_acquire.result == vk::Result::eErrorOutOfDateKHR)
        {
            ownerApp->recreateVkSwapChain();
            return;
        }
        else
            imageIndex = result_acquire.value;

        if (state.vk.sync.imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(
                state.vk.device, 1, &state.vk.sync.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }

        fillCommandBuffers(imageIndex);
        auto cb0 = m_cmdBufs->getCmdBuffer(0);

        state.vk.sync.imagesInFlight[imageIndex] = state.vk.sync.inFlightFences[m_currentFrame];

        std::array<vk::Semaphore, 1> signalSemaphores    = {state.vk.sync.renderFinishedSems[m_currentFrame]};
        std::array<vk::Semaphore, 1> waitSemaphores      = {state.vk.sync.imageAvailableSems[m_currentFrame]};
        std::array<vk::PipelineStageFlags, 1> waitStages = {
            vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo submitInfo {};
        submitInfo.setWaitSemaphoreCount(waitSemaphores.size())
            .setPWaitSemaphores(waitSemaphores.data())
            .setPWaitDstStageMask(waitStages.data())
            .setCommandBufferCount(1)
            .setPCommandBuffers(&cb0)
            .setSignalSemaphoreCount(signalSemaphores.size())
            .setPSignalSemaphores(signalSemaphores.data());

        vkResetFences(state.vk.device, 1, &state.vk.sync.inFlightFences[m_currentFrame]);

        auto iff = vk::Fence(state.vk.sync.inFlightFences[m_currentFrame]);
        if (state.vk.queues.graphics.submit(submitInfo, iff) != vk::Result::eSuccess)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Failed to submit Vulkan draw command buffer"));
        }

        std::array<vk::SwapchainKHR, 1> swapChains = {state.vk.swapChain.sc};
        vk::PresentInfoKHR presentInfo {};
        presentInfo.setWaitSemaphoreCount(signalSemaphores.size())
            .setPWaitSemaphores(signalSemaphores.data())
            .setSwapchainCount(swapChains.size())
            .setPSwapchains(swapChains.data())
            .setPImageIndices(&imageIndex);

        state.vk.queues.present.presentKHR(&presentInfo);

        m_currentFrame = (m_currentFrame + 1) % state.vk.swapChain.maxFramesInFlight;
    }

    void Renderer::fillCommandBuffers(std::size_t imageIndex)
    {
        auto& fb = state.vk.swapChain.framebuffers;
        if (!m_cmdBufs)
        {
            m_cmdBufs = CommandBuffers::create(state.vk.device,
                                               {
                                                   .cmdPool = state.vk.cmd.gfxPool,
                                               });
        }
        auto& cmdBufs = m_cmdBufs;
        // Configure render passes
        if (auto pipeline = state.vk.pipelines.mainGfx.lock())
        {
            auto& scExtent = state.vk.swapChain.extent;
            VkClearValue clearColor {0.0f, 0.0f, 0.0f, 1.0f};
            std::array<VkClearValue, 2> clearValues {};
            clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};

            {
                cmdBufs->start({.index = 0u});

                VkRenderPassBeginInfo renderPassInfo {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = pipeline->getRenderPass(),
					.framebuffer = fb[imageIndex]->getFramebuffer(),
					.renderArea = {
						.offset = {0, 0},
						.extent = scExtent,
					},
					.clearValueCount = static_cast<uint32_t>(clearValues.size()),
					.pClearValues = clearValues.data(),
				};
                vkCmdBeginRenderPass(cmdBufs->getCmdBuffer(0), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                render();

                vkCmdEndRenderPass(cmdBufs->getCmdBuffer(0));

                cmdBufs->finish();
            }
        }
    }

    void Renderer::render() { ownerApp->render(m_cmdBufs); }

} // namespace ivulk
