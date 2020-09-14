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

#include <ivulk/vk.hpp>
#include <optional>
#include <stdexcept>

#include <cstring>

namespace ivulk {

    namespace E_BufferUsage {
        constexpr vk::BufferUsageFlagBits TransferSrc  = vk::BufferUsageFlagBits::eTransferSrc;
        constexpr vk::BufferUsageFlagBits TransferDst  = vk::BufferUsageFlagBits::eTransferDst;
        constexpr vk::BufferUsageFlagBits UniformTexel = vk::BufferUsageFlagBits::eUniformTexelBuffer;
        constexpr vk::BufferUsageFlagBits StorageTexel = vk::BufferUsageFlagBits::eStorageTexelBuffer;
        constexpr vk::BufferUsageFlagBits Uniform      = vk::BufferUsageFlagBits::eUniformBuffer;
        constexpr vk::BufferUsageFlagBits Storage      = vk::BufferUsageFlagBits::eStorageBuffer;
        constexpr vk::BufferUsageFlagBits Index        = vk::BufferUsageFlagBits::eIndexBuffer;
        constexpr vk::BufferUsageFlagBits Vertex       = vk::BufferUsageFlagBits::eVertexBuffer;
        constexpr vk::BufferUsageFlagBits Indirect     = vk::BufferUsageFlagBits::eIndirectBuffer;
    } // namespace E_BufferUsage

    struct BufferInfo final
    {
        VkDeviceSize size         = 0;
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
        vk::BufferUsageFlags usage;
        VmaMemoryUsage memoryMode = E_MemoryMode::Unknown;
    };

    class Buffer : public VulkanResource<Buffer, BufferInfo, vk::Buffer, VmaAllocation>
    {
    public:
        Buffer(VkDevice device, vk::Buffer buf, VmaAllocation alloc)
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
