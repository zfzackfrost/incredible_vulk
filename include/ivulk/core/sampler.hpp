/**
 * @file sampler.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Sampler` class.
 */

#pragma once

#include <ivulk/core/vulkan_resource.hpp>
#include <ivulk/core/shader_stage.hpp>
#include <vulkan/vulkan.h>

#include <optional>

namespace ivulk {

	namespace E_SamplerFilter {
		constexpr VkFilter Nearest = VK_FILTER_NEAREST;
		constexpr VkFilter Linear  = VK_FILTER_LINEAR;
	} // namespace E_SamplerFilter
	namespace E_SamplerAddressMode {
		constexpr VkSamplerAddressMode Repeat       = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		constexpr VkSamplerAddressMode RepeatMirror = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		constexpr VkSamplerAddressMode ClampEdge    = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		constexpr VkSamplerAddressMode ClampBorder  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	} // namespace E_SamplerAddressMode

	struct SamplerInfo final
	{
		struct
		{
			VkFilter min = E_SamplerFilter::Linear;
			VkFilter mag = E_SamplerFilter::Linear;
		} filter;
		struct
		{
			VkSamplerAddressMode u = E_SamplerAddressMode::Repeat;
			VkSamplerAddressMode v = E_SamplerAddressMode::Repeat;
			VkSamplerAddressMode w = E_SamplerAddressMode::Repeat;
		} addressMode;
		struct
		{
			VkBool32 bEnable = VK_TRUE;
			float level      = 16.0f;
		} anisotropy;

		struct
		{
			VkBool32 bEnable      = VK_FALSE;
			VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
		} compare;

		struct
		{
			VkSamplerMipmapMode mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			float lodBias            = 0.0f;
			float minLod             = 0.0f;
			float maxLod             = 0.0f;
		} mips;

		uint32_t defaultBinding = 0u;
		VkShaderStageFlags stageFlags = E_ShaderStage::AllGraphics;
	};

	class Sampler : public VulkanResource<Sampler, SamplerInfo, VkSampler, VkDescriptorSetLayoutBinding>
	{
	public:
		VkSampler getSampler() { return getHandleAt<0>(); }
		VkDescriptorSetLayoutBinding getDescriptorLayoutBinding(std::optional<uint32_t> bindingIndex = {}) 
		{ 
			auto binding = getHandleAt<1>();
			if (bindingIndex.has_value())
				binding.binding = *bindingIndex;
			return binding;
		}

	private:
		friend base_t;

		Sampler(VkDevice device, VkSampler sampler, VkDescriptorSetLayoutBinding binding);

		static Sampler* createImpl(VkDevice device, SamplerInfo info);
		void destroyImpl();
	};
} // namespace ivulk
