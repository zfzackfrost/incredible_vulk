/**
 * @file graphics_pipeline.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `GraphicsPipeline` class.
 */

#pragma once
#include <ivulk/core/vulkan_resource.hpp>

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace ivulk {
	class GraphicsPipeline : public VulkanResource<GraphicsPipeline,
												   NullResourceInfo,
												   VkPipeline,
												   VkRenderPass,
												   VkPipelineLayout,
												   VkDescriptorSetLayout,
												   std::vector<VkDescriptorSet>>
	{
	public:
		GraphicsPipeline(VkDevice device,
						 VkPipeline pipeline,
						 VkRenderPass renderPass,
						 VkPipelineLayout pipelineLayout,
						 VkDescriptorSetLayout descrSetLayout,
						 std::vector<VkDescriptorSet> descrSets)
			: base_t(device,
					 handles_t {pipeline, renderPass, pipelineLayout, descrSetLayout, descrSets})
			, m_colorAttIndices {}
		{ }

		VkPipeline getPipeline() { return getHandleAt<0>(); }
		VkRenderPass getRenderPass() { return getHandleAt<1>(); }
		VkPipelineLayout getPipelineLayout() { return getHandleAt<2>(); }
		VkDescriptorSetLayout getDescriptorSetLayout() { return getHandleAt<3>(); }
		std::vector<VkDescriptorSet> getDescriptorSets() { return getHandleAt<4>(); }
		VkDescriptorSet getDescriptorSetAt(std::size_t i) { return getDescriptorSets().at(i); }

		std::vector<uint32_t> getColorAttIndices() { return m_colorAttIndices; }

	private:
		friend base_t;
		friend class App;

		std::vector<uint32_t> m_colorAttIndices;

		static GraphicsPipeline* createImpl(VkDevice, NullResourceInfo);

		void destroyImpl()
		{
			vkDestroyPipeline(getDevice(), getPipeline(), nullptr);
			vkDestroyPipelineLayout(getDevice(), getPipelineLayout(), nullptr);
			vkDestroyRenderPass(getDevice(), getRenderPass(), nullptr);
			vkDestroyDescriptorSetLayout(getDevice(), getDescriptorSetLayout(), nullptr);
		}
	};
} // namespace ivulk
