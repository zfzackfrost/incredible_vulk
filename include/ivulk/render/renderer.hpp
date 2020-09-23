/**
 * @file renderer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Renderer` class and related.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/app_state.hpp>
#include <ivulk/core/command_buffer.hpp>

#include <memory>
#include <type_traits>
namespace ivulk {
    class App;

    class Renderer : public std::enable_shared_from_this<Renderer>
    {
    public:
        using Ptr = std::shared_ptr<Renderer>;
        using Ref = std::weak_ptr<Renderer>;

        Renderer() = delete;

        template <typename Derived, typename... Args>
        static std::shared_ptr<Derived> create(App* ownerApp, Args... args)
        {
            static_assert(std::is_base_of_v<Renderer, Derived>,
                          "Template type parameter `Derived` of  `create` needs to inherit from `Renderer`");

            return std::shared_ptr<Derived>(new Derived(ownerApp, args...));
        }

        static std::weak_ptr<Renderer> current();

        void activate();

        void renderOffscreen(FramebufferInfo fbInfo);
        void renderSwapchain();
        
        void copyToSwapchain(Image::Ref colorBuf);

        virtual void drawFrame();

    protected:
        /**
         * @brief The Vulkan object a renderer will actually render to
         */
        enum class E_RenderDest : uint8_t
        {
            Undefined,
            SwapChain, ///< Render directly to swapchain (only used for simple scenes)
            Offscreen, ///< Render to offscreen images (i.e post-processing, compositing, etc.)
        };
        virtual void fillCommandBuffers(std::size_t i);
        virtual void render();

        explicit Renderer(App* ownerApp);

        App* ownerApp;
        AppState& state;
        static void makeCurrent(Renderer* newCurrent = nullptr);

        uint32_t m_currentFrame;
        CommandBuffers::Ptr m_cmdBufs;

        E_RenderDest m_dest = E_RenderDest::Undefined;

        FramebufferInfo m_fbInfo;
        Framebuffer::Ptr m_fb;
    };
} // namespace ivulk
