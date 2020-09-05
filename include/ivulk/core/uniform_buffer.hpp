/**
 * @file uniform_buffer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `UniformBufferObject` class.
 */

#pragma once

#include <ivulk/core/buffer.hpp>
#include <ivulk/core/shader_stage.hpp>

#include <typeindex>

namespace ivulk {
	struct UniformBufferObjectInfo final
	{
		VkDeviceSize size             = 0u;
		VmaMemoryUsage memoryMode     = E_MemoryMode::CpuToGpu;
		VkShaderStageFlags stageFlags = E_ShaderStage::AllGraphics;
		uint32_t binding = 0u;
	};

	class UniformBufferObject : public VulkanResource<UniformBufferObject,
													  UniformBufferObjectInfo,
													  Buffer::Ptr,
													  VkDeviceSize,
													  VkDescriptorSetLayoutBinding>
	{
	public:
		Buffer::Ref getBuffer() { return getHandleAt<0>(); }
		VkDeviceSize getSize() { return getHandleAt<1>(); }
		VkDescriptorSetLayoutBinding getDescriptorSetLayoutBinding(uint32_t bindingIndex)
		{
			auto descrBinding    = getHandleAt<2>();
			descrBinding.binding = bindingIndex;
			return descrBinding;
		}

		template <typename BufferData>
		void setUniforms(const BufferData bufData)
		{
			constexpr VkDeviceSize SZ = sizeof(BufferData);
			if (SZ != getSize())
			{
				throw std::runtime_error(utils::makeErrorMessage(
					"VK::UNIFORM",
					"Supplied buffer data does not match the size of the `UniformBufferObject`"));
			}
			if (auto buf = getBuffer().lock())
			{
				BufferData data[] = {bufData};
				buf->fillBuffer(data, SZ);
			}
		}

	private:
		friend base_t;

		UniformBufferObject(VkDevice device,
							Buffer::Ptr buffer,
							VkDeviceSize sz,
							VkDescriptorSetLayoutBinding descrBinding)
			: base_t(device, handles_t {buffer, sz, descrBinding})
		{ }

		static UniformBufferObject* createImpl(VkDevice device, UniformBufferObjectInfo createInfo)
		{
			auto buffer = Buffer::create(device,
										 {
											 .size       = createInfo.size,
											 .usage      = E_BufferUsage::Uniform,
											 .memoryMode = createInfo.memoryMode,
										 });
			VkDescriptorSetLayoutBinding descrBinding {
				.binding         = createInfo.binding,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.stageFlags = createInfo.stageFlags,
			};
			return new UniformBufferObject(device, buffer, createInfo.size, descrBinding);
		}

		void destroyImpl()
		{
			if (auto buf = getBuffer().lock())
				buf->destroy();
		}
	};
} // namespace ivulk
