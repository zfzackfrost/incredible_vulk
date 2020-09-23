/**
 * @file graphics_pipeline.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `GraphicsPipeline` class.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/vulkan_resource.hpp>

#include <ivulk/core/texture.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/vertex.hpp>

#include <optional>
#include <stdexcept>
#include <vector>
#include <ivulk/vk.hpp>

#include <boost/filesystem.hpp>

namespace ivulk {
    /**
     * @brief Information for initializing a GraphicsPipeline resource
     */
    struct GraphicsPipelineInfo final
    {
        PipelineVertexInfo vertex; ///< The vertex format for the graphics pipeline

        /**
         * @brief The paths to load SPIR-V shaders from
         */
        struct shaderPath
        {
            std::optional<boost::filesystem::path> vert = {}; ///< Vertex shader path
            std::optional<boost::filesystem::path> frag = {}; ///< Fragment shader path
            std::optional<boost::filesystem::path> tese = {}; ///< Tesselation evaluation shader path
            std::optional<boost::filesystem::path> tesc = {}; ///< Tesselation control shader path
            std::optional<boost::filesystem::path> geom = {}; ///< Geometry shader path
            std::optional<boost::filesystem::path> comp = {}; ///< Compute shader path
        } shaderPath;


        /**
         * @brief Pipeline descriptor bindings
         */
        struct descriptor
        {
            std::vector<PipelineUniformBufferBinding> uboBindings = {}; ///< Descriptor bindings for uniform buffers
            std::vector<PipelineTextureBinding> textureBindings   = {}; ///< Descriptor bindings for textures
        } descriptor;
    };

    /**
     * @brief A memory-managed resource for a Vulkan graphics pipeline
     */
    class GraphicsPipeline : public VulkanResource<GraphicsPipeline,
                                                   GraphicsPipelineInfo,
                                                   vk::Pipeline,
                                                   vk::RenderPass,
                                                   vk::PipelineLayout,
                                                   vk::DescriptorSetLayout,
                                                   std::vector<vk::DescriptorSet>>
    {
    public:
        /**
         * @brief Get the Vulkan pipeline handle
         */
        vk::Pipeline getPipeline() { return getHandleAt<0>(); }
        
        /**
         * @brief Get the Vulkan render pass handle
         */
        vk::RenderPass getRenderPass() { return getHandleAt<1>(); }
        
        /**
         * @brief Get the Vulkan pipeline layout handle
         */
        vk::PipelineLayout getPipelineLayout() { return getHandleAt<2>(); }
        
        /**
         * @brief Get the Vulkan descriptor set layout handle
         */
        vk::DescriptorSetLayout getDescriptorSetLayout() { return getHandleAt<3>(); }
        
        /**
         * @brief Get the STL vector of Vulkan descriptor set handles
         */
        std::vector<vk::DescriptorSet> getDescriptorSets() { return getHandleAt<4>(); }

        /**
         * @brief Get a Vulkan descriptor set by index
         *
         * @param i The index of the descriptor set to get
         */
        VkDescriptorSet getDescriptorSetAt(std::size_t i) { return getDescriptorSets().at(i); }

        /**
         * @brief Get an STL vector of color attachment indices
         */
        std::vector<uint32_t> getColorAttIndices() { return m_colorAttIndices; }

        /**
         * @brief Create a new graphics pipeline, and store it in this resource.
         *
         * This destroys the current pipline before creating the new one.
         *
         * @param info The parameters to use to create the new pipeline.
         */
        void recreate(GraphicsPipelineInfo info);

    private:
        friend base_t;
        friend class App;

        GraphicsPipeline(vk::Device device,
                         vk::Pipeline pipeline,
                         vk::RenderPass renderPass,
                         vk::PipelineLayout pipelineLayout,
                         vk::DescriptorSetLayout descrSetLayout,
                         std::vector<vk::DescriptorSet> descrSets);

        std::vector<uint32_t> m_colorAttIndices;

        static GraphicsPipeline* createImpl(VkDevice device, GraphicsPipelineInfo info);

        void destroyImpl();

        static std::vector<char> readSPIRVFile(const boost::filesystem::path& fpath);
        static vk::ShaderModule createShaderModule(VkDevice device,
                                                 const std::vector<char>& shaderCode,
                                                 const boost::filesystem::path& assetPath);
    };

} // namespace ivulk
