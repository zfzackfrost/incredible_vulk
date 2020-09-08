/**
 * @file texture.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `PipelineTextureBinding` struct.
 */

#pragma once

#include <ivulk/core/image.hpp>
#include <ivulk/core/sampler.hpp>

namespace ivulk {

	struct PipelineTextureBinding final
	{
		Image::Ref image;
		Sampler::Ref sampler;
		uint32_t binding;

		VkDescriptorSetLayoutBinding getDescriptorSetLayoutBinding()
		{
			VkDescriptorSetLayoutBinding result;
			if (auto r = sampler.lock())
			{
				result = r->getDescriptorLayoutBinding(binding);
			}
			return result;
		}

		VkImageView getImageView() const
		{
			if (auto r = image.lock())
			{
				return r->getImageView();
			}
			return VK_NULL_HANDLE;
		}
		VkSampler getSampler() const
		{
			if (auto r = sampler.lock())
			{
				return r->getSampler();
			}
			return VK_NULL_HANDLE;
		}
	};

} // namespace ivulk
