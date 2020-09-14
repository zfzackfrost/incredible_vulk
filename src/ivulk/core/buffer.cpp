#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/command_buffer.hpp>

namespace ivulk {

    Buffer* Buffer::createImpl(VkDevice device, BufferInfo info)
    {
        auto state     = App::current()->getState();
        auto allocator = state.vk.allocator;

        VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        };
        bufferInfo.size  = info.size;
        bufferInfo.usage = info.usage;
        VkBuffer buffer;

        VmaAllocationCreateInfo allocInfo {
            .usage = info.memoryMode,
        };
        VmaAllocation alloc = VK_NULL_HANDLE;
        if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &alloc, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan buffer"));
        }

        // Return value
        auto* ret   = new Buffer(device, buffer, alloc);
        ret->m_size = info.size;
        return ret;
    }

    void Buffer::destroyImpl()
    {
        auto state     = App::current()->getState();
        auto allocator = state.vk.allocator;
        vmaDestroyBuffer(allocator, getBuffer(), getAllocation());
    }

    uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        auto state = App::current()->getState();

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(state.vk.physicalDevice, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1u << i))
                && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error(
            utils::makeErrorMessage("VK::MEM", "Failed to find suitable memory type for buffer."));
    }

    void Buffer::fillBuffer(const void* data, VkDeviceSize sz, std::optional<uint32_t> newCount)
    {
        auto state     = App::current()->getState();
        auto allocator = state.vk.allocator;
        void* mappedData;
        vmaMapMemory(allocator, getAllocation(), &mappedData);
        std::memcpy(mappedData, data, sz);
        vmaUnmapMemory(allocator, getAllocation());
        if (newCount.has_value())
            m_count = *newCount;
    }

    void Buffer::copyFromBuffer(Buffer::Ref srcBuf, VkDeviceSize size)
    {
        auto state = App::current()->getState();
        if (auto sb = srcBuf.lock())
        {
            const VkBufferCopy cpyRegion {
                .srcOffset = 0,
                .dstOffset = 0,
                .size      = size,
            };
            auto cmdBufs = CommandBuffers::create(getDevice(),
                                                  {
                                                      .cmdPool = state.vk.cmd.gfxPool,
                                                  });
            auto cb0     = cmdBufs->getCmdBuffer(0);
            cmdBufs->start(0, _flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            vkCmdCopyBuffer(cb0, sb->getBuffer(), getBuffer(), 1, &cpyRegion);
            cmdBufs->finish();

            m_count = sb->m_count;
            VkSubmitInfo submitInfo {
                .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers    = &cb0,
            };
            vkQueueSubmit(state.vk.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(state.vk.queues.graphics);
        }
    }
} // namespace ivulk
