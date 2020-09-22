/**
 * @file command_buffer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `CommandBuffer` class.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/vulkan_resource.hpp>

#include <ivulk/core/shader_stage.hpp>
#include <ivulk/utils/keywords.hpp>
#include <ivulk/utils/messages.hpp>

#include <glm/glm.hpp>
#include <ivulk/vk.hpp>
#include <optional>
#include <stdexcept>
#include <vector>

namespace ivulk {
    class Buffer;
    class GraphicsPipeline;

    /**
     * @brief Information for initializing a CommandBuffers resource
     */
    struct CommandBuffersCreateInfo
    {
        vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary; ///< Vulkan command buffer level.
        vk::CommandPool cmdPool; ///< The Vulkan command buffer pool to allocate from
        uint32_t count = 1;      ///< The number of command buffers to allocate
    };

    /**
     * @brief A memory-managed resource for a group of Vulkan command buffers
     */
    class CommandBuffers : public VulkanResource<CommandBuffers,
                                                 CommandBuffersCreateInfo,
                                                 vk::CommandPool,
                                                 std::vector<vk::CommandBuffer>>
    {
    public:
        CommandBuffers(VkDevice device, vk::CommandPool pool, std::vector<vk::CommandBuffer> buffers)
            : base_t(device, handles_t {pool, buffers})
        { }

        /**
         * @brief Get the Vulkan command pool used to allocate the command buffers.
         */
        vk::CommandPool getCmdPool() { return getHandleAt<0>(); }

        /**
         * @brief Get a command buffer from this group by index.
         *
         * @param i The index of the command buffer to get
         */
        vk::CommandBuffer getCmdBuffer(std::size_t i) { return getHandleAt<1>()[i]; }

        /**
         * @brief Get an STL vector of all the command buffers in this group.
         */
        std::vector<vk::CommandBuffer> getCmdBuffers() { return getHandleAt<1>(); }

        /**
         * @brief Optional arguments for the `start` method.
         */
        struct StartCallInfo
        {
            std::size_t index; ///< The index of the command buffer to start recording to.
            vk::CommandBufferUsageFlags flags = vk::CommandBufferUsageFlagBits::
                eSimultaneousUse; ///< The Vulkan command buffer usage flags.
        };
        /** 
         * @brief Set the current command buffer by index and start recording to it.
         *
         * @param callInfo The optional arguments structure.
         */
        void start(const StartCallInfo&& callInfo) { startImpl(callInfo.index, callInfo.flags); }

        /**
         * @brief Finish recording to the current command buffer
         */
        void finish();

        /**
         * @brief Optional arguments for the `draw` method.
         */
        struct DrawCallInfo
        {
            std::weak_ptr<Buffer> vertexBuffer; ///< The vertex buffer to use for drawing
            std::weak_ptr<Buffer> indexBuffer;  ///< The index buffer to use for drawing
            uint32_t vertices      = 0u;        ///< Override the number of vertices to draw
            uint32_t instances     = 1u;        ///< The number of instances for instanced rendering
            uint32_t firstVertex   = 0u;        ///< The index of the first vertex to draw
            uint32_t firstInstance = 0u;        ///< The index of the first instance to draw
        };

        /** 
         * @brief Bind vertex/index buffers and draw from them.
         *
         * @param callInfo The optional arguments structure.
         */
        void draw(const DrawCallInfo&& callInfo)
        {
            drawImpl(callInfo.vertexBuffer,
                     callInfo.indexBuffer,
                     callInfo.vertices,
                     callInfo.instances,
                     callInfo.firstVertex,
                     callInfo.firstInstance);
        }

        /**
         * @brief Optional arguments for the `clearAttachments` method.
         */
        struct ClearAttachmentsCallInfo
        {
            glm::vec4 color            = {0, 0, 0, 1}; ///< The clear color
            std::optional<float> depth = 0.0f;         ///< The clear depth, if any
        };

        /**
         * @brief Clear the attachments for a graphics pipeline
         *
         * @param pipeline The pipeline we are working with
         * @param callInfo The option arguments structure
         */
        void clearAttachments(std::weak_ptr<GraphicsPipeline> pipeline,
                              const ClearAttachmentsCallInfo&& callInfo)
        {
            clearAttachmentsImpl(pipeline, callInfo.color);
        }

        /**
         * @brief Bind a graphics pipeline
         *
         * @param pipeline The pipeline to bind
         */
        void bindPipeline(std::weak_ptr<GraphicsPipeline> pipeline) { bindPipelineImpl(pipeline); }

        /**
         * @brief Optional arguments structure for `pushConstants` method.
         */
        struct PushConstantsCallInfo
        {
            vk::DeviceSize offset
                = 0u; ///< the start offset of the push constant range to update, in units of bytes
            VkShaderStageFlags stageFlags = E_ShaderStage::
                All; ///< the shader stages that will use the push constants in the updated range
        };

        /**
         * @brief Update the values of push constants.
         *
         * @param data The data to update the push constants with
         * @param layout The pipeline layout used to program the push constant updates
         * @param size The size of the push constant range to update, in units of bytes
         */
        void pushConstants(const void* data,
                           vk::PipelineLayout layout,
                           vk::DeviceSize size,
                           const PushConstantsCallInfo&& callInfo)
        {
            pushConstantsImpl(data, layout, callInfo.stageFlags, callInfo.offset, size);
        }

    private:
        friend base_t;

        static CommandBuffers* createImpl(VkDevice device, CommandBuffersCreateInfo createInfo)
        {
            vk::CommandBufferAllocateInfo allocInfo {};
            allocInfo.setCommandPool(createInfo.cmdPool);
            allocInfo.setLevel(createInfo.level);
            allocInfo.setCommandBufferCount(createInfo.count);

            std::vector<VkCommandBuffer> commandBuffers(createInfo.count);
            VkCommandBufferAllocateInfo ai = allocInfo;
            if (vkAllocateCommandBuffers(device, &ai, commandBuffers.data()) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan command buffer(s)"));
            }

            return new CommandBuffers(
                device,
                createInfo.cmdPool,
                std::vector<vk::CommandBuffer>(commandBuffers.begin(), commandBuffers.end()));
        }

        void destroyImpl() { }

        void startImpl(std::size_t index, vk::CommandBufferUsageFlags flags);
        void drawImpl(std::weak_ptr<Buffer> vertexBuffer,
                      std::weak_ptr<Buffer> indexBuffer,
                      uint32_t vertices,
                      uint32_t instances,
                      uint32_t firstVertex,
                      uint32_t firstInstance);
        void clearAttachmentsImpl(std::weak_ptr<GraphicsPipeline> pipeline, glm::vec4 color);

        void bindPipelineImpl(std::weak_ptr<GraphicsPipeline> pipeline);

        void pushConstantsImpl(const void* data,
                               VkPipelineLayout layout,
                               VkShaderStageFlags stages,
                               VkDeviceSize offset,
                               VkDeviceSize size);

        std::optional<std::size_t> m_currentIdx = {};
    };
} // namespace ivulk
