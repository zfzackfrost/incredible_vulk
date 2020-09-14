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
    
    struct FramebufferInfo final
    {
        std::variant<GraphicsPipeline::Ref, GraphicsPipeline::Ptr, vk::RenderPass> renderContext {};
        std::vector<std::variant<Image::Ref, Image::Ptr, vk::ImageView>> attachments;
        uint32_t width  = 0u;
        uint32_t height = 0u;
        uint32_t layers = 1u;
    };

    class Framebuffer : public VulkanResource<Framebuffer, FramebufferInfo, vk::Framebuffer>
    {
    public:
        vk::Framebuffer getFramebuffer() { return getHandleAt<0>(); }

    private:
        friend base_t;

        Framebuffer(VkDevice device, VkFramebuffer fbuf);

        static Framebuffer* createImpl(const VkDevice device, const FramebufferInfo info);
        void destroyImpl();
    };
} // namespace ivulk
