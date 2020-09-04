/**
 * @file graphics_pipeline.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Buffer` class.
 */

#pragma once
#include <ivulk/core/app.hpp>
#include <ivulk/core/vulkan_resource.hpp>
#include <ivulk/utils/messages.hpp>

#include <stdexcept>
#include <vulkan/vulkan.h>

#include <cstring>

namespace ivulk {

	namespace E_BufferUsage {
		enum Type
		{
			TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			UniformTexel = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
			StorageTexel = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
			Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			Indirect = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		};
	}

	enum class E_BufferMemoryMode : uint8_t
	{
		CPUShared = 0u,
		GPUOnly = 1u,
	};

	struct BufferInfo final
	{
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		E_BufferMemoryMode memoryMode = E_BufferMemoryMode::CPUShared;
	};
	class Buffer : public VulkanResource<Buffer, BufferInfo, VkBuffer, VkDeviceMemory>
	{
	public:
		Buffer(VkDevice device, VkBuffer buf, VkDeviceMemory mem)
			: base_t(device, handles_t {buf, mem})
		{ }

		VkBuffer getBuffer() { return getHandleAt<0>(); }
		VkDeviceMemory getDeviceMemory() { return getHandleAt<1>(); }
		uint32_t getCount() { return m_count; }

		template <typename T>
		void fillBuffer(T* elems, std::size_t num)
		{
			const auto sz = sizeof(T) * num;
			void* data;
			vkMapMemory(getDevice(), getDeviceMemory(), 0, sz, 0, &data);
			std::memcpy(data, elems, sz);
			vkUnmapMemory(getDevice(), getDeviceMemory());
			m_count = num;
		}

	private:
		friend base_t;

		uint32_t m_count = 0;

		static Buffer* createImpl(VkDevice device, BufferInfo info)
		{
			VkBufferCreateInfo createInfo {
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = info.size,
				.usage = info.usage,
				.sharingMode = info.sharingMode,
			};
			VkBuffer buffer;
			VkDeviceMemory deviceMem = VK_NULL_HANDLE;

			if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
				throw std::runtime_error(
					utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan buffer"));

			VkMemoryRequirements memReq;
			vkGetBufferMemoryRequirements(device, buffer, &memReq);

			constexpr VkMemoryPropertyFlags CPU_SHARED_FLAGS = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			constexpr VkMemoryPropertyFlags GPU_ONLY_FLAGS = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VkMemoryPropertyFlags memFlags = (info.memoryMode == E_BufferMemoryMode::CPUShared)
				? CPU_SHARED_FLAGS
				: GPU_ONLY_FLAGS;

			const VkMemoryAllocateInfo allocInfo {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = memReq.size,
				.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, memFlags),
			};
			if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMem) != VK_SUCCESS)
				throw std::runtime_error(
					utils::makeErrorMessage("VK::MEM", "Failed to allocate memory for Vulkan buffer"));

			if (vkBindBufferMemory(device, buffer, deviceMem, 0) != VK_SUCCESS)
				throw std::runtime_error(
					utils::makeErrorMessage("VK::BUF", "Failed to bind memory to new Vulkan buffer"));

			return new Buffer(device, buffer, deviceMem);
		}

		void destroyImpl()
		{
			vkDestroyBuffer(getDevice(), getBuffer(), nullptr);
			auto deviceMem = getDeviceMemory();
			if (deviceMem != VK_NULL_HANDLE)
				vkFreeMemory(getDevice(), deviceMem, nullptr);
		}

		static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
	};
} // namespace ivulk
