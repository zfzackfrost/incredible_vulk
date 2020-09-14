#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/utils/messages.hpp>

#include <algorithm>
#include <stdexcept>

namespace ivulk {
    VkSemaphore createVkSemaphore(VkDevice device)
    {
        VkSemaphore semaphore;
        VkSemaphoreCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        if (vkCreateSemaphore(device, &createInfo, nullptr, &semaphore) != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan semaphore"));
        }
        return semaphore;
    }
    VkFence createVkFence(VkDevice device)
    {
        VkFence fence;
        VkFenceCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        if (vkCreateFence(device, &createInfo, nullptr, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan fence"));
        }
        return fence;
    }
    void App::createVkSyncObjects()
    {
        auto& sync        = state.vk.sync;
        auto genSemaphore = [&state = state]() { return createVkSemaphore(state.vk.device); };
        auto genFence     = [&state = state]() { return createVkFence(state.vk.device); };

        sync.imageAvailableSems.resize(m_initArgs.vk.maxFramesInFlight);
        std::generate(sync.imageAvailableSems.begin(), sync.imageAvailableSems.end(), genSemaphore);

        sync.renderFinishedSems.resize(m_initArgs.vk.maxFramesInFlight);
        std::generate(sync.renderFinishedSems.begin(), sync.renderFinishedSems.end(), genSemaphore);

        sync.inFlightFences.resize(m_initArgs.vk.maxFramesInFlight);
        std::generate(sync.inFlightFences.begin(), sync.inFlightFences.end(), genFence);

        state.vk.sync.imagesInFlight.resize(state.vk.swapChain.images.size(), VK_NULL_HANDLE);
    }

} // namespace ivulk
