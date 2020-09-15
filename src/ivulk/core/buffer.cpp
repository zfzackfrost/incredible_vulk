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

        vk::BufferCreateInfo bufferInfo {};
        bufferInfo.setSize(info.size);
        bufferInfo.setUsage(info.usage);
        bufferInfo.setSharingMode(info.sharingMode);
        VkBuffer buffer;

        VmaAllocationCreateInfo allocInfo {
            .usage = info.memoryMode,
        };
        VkBufferCreateInfo bi = bufferInfo;
        VmaAllocation alloc   = VK_NULL_HANDLE;
        if (vmaCreateBuffer(allocator, &bi, &allocInfo, &buffer, &alloc, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan buffer"));
        }

        // Return value
        auto* ret   = new Buffer(device, vk::Buffer(buffer), alloc);
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
            vk::BufferCopy cpyRegion {};
            cpyRegion.setSrcOffset(0);
            cpyRegion.setDstOffset(0);
            cpyRegion.setSize(size);

            auto cmdBufs = CommandBuffers::create(getDevice(),
                                                  {
                                                      .cmdPool = state.vk.cmd.gfxPool,
                                                  });

            auto cb0 = cmdBufs->getCmdBuffer(0);

            cmdBufs->start(0, _flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            cb0.copyBuffer(sb->getBuffer(), getBuffer(), 1, &cpyRegion);
            cmdBufs->finish();

            vk::Queue q = state.vk.queues.graphics;
            m_count     = sb->m_count;
            vk::SubmitInfo submitInfo {};
            submitInfo.setCommandBufferCount(1);
            submitInfo.setPCommandBuffers(&cb0);

            q.submit(1, &submitInfo, nullptr);
            q.waitIdle();
        }
    }
} // namespace ivulk
