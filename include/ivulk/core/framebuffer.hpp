/**
 * @file framebuffer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Framebuffer` class.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/image.hpp>
#include <ivulk/core/vulkan_resource.hpp>

#include <ivulk/vk.hpp>

#include <variant>

namespace ivulk {

    /**
     * @brief Information for initializing a Framebuffer resource
     */
    struct FramebufferInfo final
    {
        /**
         * @brief The render context
         *
         * The source used to retrieve a Vulkan render pass handle.
         */
        std::variant<GraphicsPipeline::Ref, GraphicsPipeline::Ptr, vk::RenderPass> renderContext {};

        /**
         * @brief The framebuffer attachments
         *
         * Each vector element is used to retrieve a Vulkan image view handle.
         */
        std::vector<std::variant<Image::Ref, Image::Ptr, vk::ImageView>> attachments;

        uint32_t width  = 0u; ///< The framebuffer width
        uint32_t height = 0u; ///< The fraembuffer height
        uint32_t layers = 1u; ///< The number of layers in the framebuffer
    };

    /**
     * @brief A memory-managed resource for a Vulkan framebuffer
     */
    class Framebuffer : public VulkanResource<Framebuffer, FramebufferInfo, vk::Framebuffer>
    {
    public:
        /**
         * @brief Get the Vulkan framebuffer handle
         */
        vk::Framebuffer getFramebuffer() { return getHandleAt<0>(); }

    private:
        friend base_t;

        Framebuffer(VkDevice device, VkFramebuffer fbuf);

        static Framebuffer* createImpl(const VkDevice device, const FramebufferInfo info);
        void destroyImpl();
    };
} // namespace ivulk
