/**
 * @file graphics_pipeline.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Buffer` class.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/vma.hpp>
#include <ivulk/core/vulkan_resource.hpp>
#include <ivulk/utils/messages.hpp>

#include <optional>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include <cstring>

namespace ivulk {

    namespace E_BufferUsage {
        enum Type
        {
            TransferSrc  = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            TransferDst  = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            UniformTexel = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
            StorageTexel = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
            Uniform      = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            Storage      = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            Index        = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            Vertex       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            Indirect     = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        };
    }

    struct BufferInfo final
    {
        VkDeviceSize size         = 0;
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VkBufferUsageFlags usage  = 0;
        VmaMemoryUsage memoryMode = E_MemoryMode::Unknown;
    };

    class Buffer : public VulkanResource<Buffer, BufferInfo, VkBuffer, VmaAllocation>
    {
    public:
        Buffer(VkDevice device, VkBuffer buf, VmaAllocation alloc)
            : base_t(device, handles_t {buf, alloc})
        { }

        VkBuffer getBuffer() { return getHandleAt<0>(); }
        VmaAllocation getAllocation() { return getHandleAt<1>(); }
        uint32_t getCount() { return m_count; }
        VkDeviceSize getSize() { return m_size; }

        void fillBuffer(const void* data, VkDeviceSize sz, std::optional<uint32_t> newCount = {});

        void copyFromBuffer(Buffer::Ref srcBuf, VkDeviceSize size);

    private:
        friend base_t;

        VkDeviceSize m_size = 0;
        uint32_t m_count    = 0;

        static Buffer* createImpl(VkDevice device, BufferInfo info);
        void destroyImpl();
        static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
} // namespace ivulk
