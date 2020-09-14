#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/render/renderer.hpp>

#include <ivulk/core/app.hpp>

#include <ivulk/vk.hpp>

namespace ivulk {
    Renderer::Renderer(App* ownerApp)
        : state(ownerApp->state)
    {
    }

    Renderer* s_current = nullptr;

    Renderer* Renderer::current() 
    {
        return s_current;
    }

    void Renderer::makeCurrent(Renderer* newCurrent)
    {
        s_current = newCurrent;
    }

    void Renderer::drawFrame()
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
    }

} // namespace ivulk
