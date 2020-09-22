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

    /**
     * @brief Information for initializing a Buffer resource
     */
    struct BufferInfo final
    {
        VkDeviceSize size           = 0;                           ///< The size of the buffer in bytes
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive; ///< The Vulkan sharing mode of the buffer
        vk::BufferUsageFlags usage;                        ///< Flags indicating how the buffer will be used
        VmaMemoryUsage memoryMode = E_MemoryMode::Unknown; ///< The memory mode for the buffer
    };

    /**
     * @brief A memory-managed resource for a Vulkan buffer
     */
    class Buffer : public VulkanResource<Buffer, BufferInfo, vk::Buffer, VmaAllocation>
    {
    public:
        Buffer(VkDevice device, vk::Buffer buf, VmaAllocation alloc)
            : base_t(device, handles_t {buf, alloc})
        { }

        /**
         * @brief Get the Vulkan buffer handle
         */
        vk::Buffer getBuffer() { return getHandleAt<0>(); }

        /**
         * @brief Get the VMA allocation info for this buffer.
         */
        VmaAllocation getAllocation() { return getHandleAt<1>(); }

        /**
         * @brief Get the number of items allocated in this buffer.
         *
         * Typically used for a vertex count for drawing
         * when this is a Vertex Buffer.
         */
        uint32_t getCount() { return m_count; }

        /**
         * @brief Get the size in bytes of this buffer
         */
        VkDeviceSize getSize() { return m_size; }

        /**
         * @brief Fill the buffer with arbitrary data.
         *
         * Additionally, set the count value if a value is provided.
         * @param data Void data to copy into the buffer.
         * @param sz The number of bytes from `data` to copy into the buffer.
         *           Must be less than or equal to the size of the buffer.
         * @param newCount Optional. If provided, the buffer's count value will be set to this.
         */
        void fillBuffer(const void* data, VkDeviceSize sz, std::optional<uint32_t> newCount = {});

        /**
         * @brief Copy the contents of another buffer into this buffer.
         *
         * @param srcBuf The buffer to copy
         * @param size The number of bytes to copy from `srcBuf`
         * @param copyCount Optional. If `true`, set this buffer's count value that of `srcBuf`. Otherwise
         *                  This buffer's count value is unchanged. Default is `true`.
         */
        void copyFromBuffer(Buffer::Ref srcBuf, VkDeviceSize size, bool copyCount = true);

    private:
        friend base_t;

        VkDeviceSize m_size = 0;
        uint32_t m_count    = 0;

        static Buffer* createImpl(VkDevice device, BufferInfo info);
        void destroyImpl();
        static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
} // namespace ivulk
