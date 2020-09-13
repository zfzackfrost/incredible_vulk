#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>

#include <ivulk/utils/messages.hpp>

namespace ivulk {
    void CommandBuffers::startImpl(std::size_t index, VkCommandBufferUsageFlags flags)
    {
        if (m_currentIdx.has_value())
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Command buffer recording already started"));

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext            = nullptr;
        beginInfo.flags            = flags;
        beginInfo.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(getCmdBuffer(index), &beginInfo) != VK_SUCCESS)
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Failed to start command buffer recording"));
        m_currentIdx = index;
    }
    void CommandBuffers::finish()
    {
        if (!m_currentIdx.has_value())
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));
        m_currentIdx = {};
        if (vkEndCommandBuffer(getCmdBuffer(*m_currentIdx)) != VK_SUCCESS)
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Failed to finish command buffer recording"));
    }

    void CommandBuffers::drawImpl(std::weak_ptr<Buffer> vertexBuffer,
                                  std::weak_ptr<Buffer> indexBuffer,
                                  uint32_t vertices,
                                  uint32_t instances,
                                  uint32_t firstVertex,
                                  uint32_t firstInstance)
    {
        if (!m_currentIdx.has_value())
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

        auto count     = vertices;
        bool isIndexed = false;
        if (auto vbuf = vertexBuffer.lock())
        {
            VkBuffer buffers[]     = {vbuf->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(getCmdBuffer(*m_currentIdx), 0, 1, buffers, offsets);
            if (count == 0)
                count = vbuf->getCount();
            if (auto ibuf = indexBuffer.lock())
            {
                vkCmdBindIndexBuffer(getCmdBuffer(*m_currentIdx), ibuf->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
                isIndexed = true;
                count     = ibuf->getCount();
            }
        }
        if (isIndexed)
            vkCmdDrawIndexed(getCmdBuffer(*m_currentIdx), count, 1, 0, 0, 0);
        else
            vkCmdDraw(getCmdBuffer(*m_currentIdx), count, instances, firstVertex, firstInstance);
    }

    void CommandBuffers::clearAttachmentsImpl(std::weak_ptr<GraphicsPipeline> pipeline, glm::vec4 color)
    {
        if (!m_currentIdx.has_value())
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

        if (auto pl = pipeline.lock())
        {
            auto colorIndices = pl->getColorAttIndices();
            std::vector<VkClearAttachment> clearAtts;
            for (auto& ci : colorIndices)
            {
                clearAtts.push_back(
                    {.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                     .colorAttachment = ci,
                     .clearValue      = {.color = {.float32 = {color.r, color.g, color.b, color.a}}}});
            }
            VkClearRect rect {
                .rect = {.offset = {0, 0}, .extent = App::current()->getState().vk.swapChain.extent},
                .baseArrayLayer = 0,
                .layerCount     = 1,
            };
            vkCmdClearAttachments(getCmdBuffer(*m_currentIdx), clearAtts.size(), clearAtts.data(), 1, &rect);
        }
    }

    void CommandBuffers::bindPipelineImpl(std::weak_ptr<GraphicsPipeline> pipeline)
    {
        if (!m_currentIdx.has_value())
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

        if (auto pl = pipeline.lock())
        {
            vkCmdBindPipeline(
                getCmdBuffer(*m_currentIdx), VK_PIPELINE_BIND_POINT_GRAPHICS, pl->getPipeline());

            if (pl->getDescriptorSets().size() > 0)
            {
                auto descrSet = pl->getDescriptorSetAt(*m_currentIdx);
                vkCmdBindDescriptorSets(getCmdBuffer(*m_currentIdx),
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pl->getPipelineLayout(),
                                        0,
                                        1,
                                        &descrSet,
                                        0,
                                        nullptr);
            }
        }
    }

    void CommandBuffers::pushConstantsImpl(const void* data, VkPipelineLayout layout, VkShaderStageFlags stages, VkDeviceSize offset, VkDeviceSize size)
    {
        if (!m_currentIdx.has_value())
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CMD", "Command buffer recording not started"));

        vkCmdPushConstants(getCmdBuffer(*m_currentIdx), layout, stages, offset, size, data);
    }

} // namespace ivulk
