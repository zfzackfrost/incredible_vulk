/**
 * @file graphics_pipeline.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `GraphicsPipeline` class.
 */

#pragma once
#include <ivulk/core/vulkan_resource.hpp>

#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/vertex.hpp>

#include <stdexcept>
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>

#include <boost/filesystem.hpp>

namespace ivulk {
	struct GraphicsPipelineInfo final
	{
		PipelineVertexInfo vertex;

		struct
		{
			std::optional<boost::filesystem::path> vert = {};
			std::optional<boost::filesystem::path> frag = {};
			std::optional<boost::filesystem::path> tese = {};
			std::optional<boost::filesystem::path> tesc = {};
			std::optional<boost::filesystem::path> geom = {};
			std::optional<boost::filesystem::path> comp = {};
		} shaderPath;

		struct
		{
			std::vector<PipelineUniformBufferBinding> uboBindings = {};
			std::vector<PipelineTextureBinding> textureBindings = {};
		} descriptor;
	};

	class GraphicsPipeline : public VulkanResource<GraphicsPipeline,
												   GraphicsPipelineInfo,
												   VkPipeline,
												   VkRenderPass,
												   VkPipelineLayout,
												   VkDescriptorSetLayout,
												   std::vector<VkDescriptorSet>>
	{
	public:

		VkPipeline getPipeline() { return getHandleAt<0>(); }
		VkRenderPass getRenderPass() { return getHandleAt<1>(); }
		VkPipelineLayout getPipelineLayout() { return getHandleAt<2>(); }
		VkDescriptorSetLayout getDescriptorSetLayout() { return getHandleAt<3>(); }
		std::vector<VkDescriptorSet> getDescriptorSets() { return getHandleAt<4>(); }
		VkDescriptorSet getDescriptorSetAt(std::size_t i) { return getDescriptorSets().at(i); }

		std::vector<uint32_t> getColorAttIndices() { return m_colorAttIndices; }

		void recreate(GraphicsPipelineInfo info);

	private:
		friend base_t;
		friend class App;
		
		GraphicsPipeline(VkDevice device,
						 VkPipeline pipeline,
						 VkRenderPass renderPass,
						 VkPipelineLayout pipelineLayout,
						 VkDescriptorSetLayout descrSetLayout,
						 std::vector<VkDescriptorSet> descrSets);

		std::vector<uint32_t> m_colorAttIndices;

		static GraphicsPipeline* createImpl(VkDevice device, GraphicsPipelineInfo info);

		void destroyImpl();


		static std::vector<char> readSPIRVFile(const boost::filesystem::path& fpath);
		static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& shaderCode,
											   const boost::filesystem::path& assetPath);
	};

} // namespace ivulk
