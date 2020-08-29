/**
 * @file graphics_pipeline.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `GraphicsPipeline` class.
 */

#pragma once
#include <ivulk/core/vulkan_resource.hpp>

#include <vulkan/vulkan.h>

namespace ivulk {
	class GraphicsPipeline : public VulkanResource<GraphicsPipeline, VkPipeline, VkRenderPass, VkPipelineLayout>
	{
	public:
		GraphicsPipeline(VkDevice device, VkPipeline pipeline, VkRenderPass renderPass, VkPipelineLayout pipelineLayout)
			: base_t(device, handles_t {pipeline, renderPass, pipelineLayout})
		{ }

		VkPipeline getPipeline() { return getHandleAt<0>(); }
		VkRenderPass getRenderPass() { return getHandleAt<1>(); }
		VkPipelineLayout getPipelineLayout() { return getHandleAt<2>(); }

		void destroyImpl()
		{
			vkDestroyPipeline(getDevice(), getPipeline(), nullptr);
			vkDestroyPipelineLayout(getDevice(), getPipelineLayout(), nullptr);
			vkDestroyRenderPass(getDevice(), getRenderPass(), nullptr);
		}
	};
} // namespace ivulk
