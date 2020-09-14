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
    struct CommandBuffersCreateInfo
    {
        vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary;
        vk::CommandPool cmdPool;
        uint32_t count = 1;
    };
    class CommandBuffers : public VulkanResource<CommandBuffers,
                                                 CommandBuffersCreateInfo,
                                                 vk::CommandPool,
                                                 std::vector<vk::CommandBuffer>>
    {
    public:
        CommandBuffers(VkDevice device, vk::CommandPool pool, std::vector<vk::CommandBuffer> buffers)
            : base_t(device, handles_t {pool, buffers})
        { }

        vk::CommandPool getCmdPool() { return getHandleAt<0>(); }
        vk::CommandBuffer getCmdBuffer(std::size_t i) { return getHandleAt<1>()[i]; }
        std::vector<vk::CommandBuffer> getCmdBuffers() { return getHandleAt<1>(); }

        // clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), start, tag, 
			(required
				(index, (std::size_t))
			)
			(optional
				(flags, (vk::CommandBufferUsageFlags), vk::CommandBufferUsageFlagBits::eSimultaneousUse)
			)
		)
        // clang-format on
        {
            startImpl(index, flags);
        }

        void finish();

        // clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), draw, tag, 
			(optional
				(vertexBuffer,  *, std::weak_ptr<Buffer>())
				(indexBuffer,   *, std::weak_ptr<Buffer>())
				(vertices,      *,                      0u)
				(instances,     *,                      1u)
				(firstVertex,   *,                      0u)
				(firstInstance, *,                      0u)
			)
		)
        // clang-format on
        {
            drawImpl(vertexBuffer, indexBuffer, vertices, instances, firstVertex, firstInstance);
        }

        // clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), clearAttachments, tag, 
			(required
				(pipeline, *)
			)
			(optional
				(color, *, glm::vec4(0,0,0,1))
			)
		)
        // clang-format on
        {
            clearAttachmentsImpl(pipeline, color);
        }

        // clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), bindPipeline, tag, 
			(required
				(pipeline, *)
			)
		)
        // clang-format on
        {
            bindPipelineImpl(pipeline);
        }

        // clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION((void), pushConstants, tag, 
			(required
				(data, *)
                (layout, *)
			)
            (optional
                (offset, *, 0u)
                (size, *, 0u)
                (stageFlags, *, E_ShaderStage::All)
            )
		)
        // clang-format on
        {
            pushConstantsImpl(data, layout, stageFlags, offset, size);
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
