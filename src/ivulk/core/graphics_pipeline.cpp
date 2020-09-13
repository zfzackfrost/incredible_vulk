#include <ivulk/core/graphics_pipeline.hpp>

#include <ivulk/core/app.hpp>

namespace ivulk {

    namespace fs = boost::filesystem;

    void GraphicsPipeline::recreate(GraphicsPipelineInfo info)
    {
        auto* tmpPipeline = createImpl(getDevice(), info);
        destroy();
        handles = tmpPipeline->handles;
        setDestroyed(false);
        tmpPipeline->setDestroyed(true);
        delete tmpPipeline;
    }

    std::vector<char> GraphicsPipeline::readSPIRVFile(const fs::path& fpath)
    {
        auto p = fpath.lexically_normal();
        std::ifstream f(p, std::ios::ate | std::ios::binary);
        if (!f.is_open())
        {
            std::string description = "Failed to open file for reading: `";
            description += p.string() + "`";
            throw std::runtime_error(utils::makeErrorMessage("FILE", description));
        }
        auto fileSize = f.tellg();
        std::vector<char> buffer(fileSize);
        f.seekg(0);
        f.read(buffer.data(), fileSize);
        f.close();
        return buffer;
    }

    VkShaderModule GraphicsPipeline::createShaderModule(VkDevice device,
                                                        const std::vector<char>& shaderCode,
                                                        const fs::path& assetPath)
    {
        VkShaderModuleCreateInfo createInfo {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = shaderCode.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data()),
        };
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            std::string description = "Failed to create Vulkan shader module from shader: `";
            description += assetPath.string() + "`";
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", description));
        }
        else if (App::current()->getPrintDbg())
        {
            std::string description = "Created Vulkan shader module from shader: `";
            description += assetPath.string() + "`";
            std::cout << utils::makeSuccessMessage("VK::CREATE", description) << std::endl;
        }
        return shaderModule;
    }

    GraphicsPipeline::GraphicsPipeline(VkDevice device,
                                       VkPipeline pipeline,
                                       VkRenderPass renderPass,
                                       VkPipelineLayout pipelineLayout,
                                       VkDescriptorSetLayout descrSetLayout,
                                       std::vector<VkDescriptorSet> descrSets)
        : base_t(device, handles_t {pipeline, renderPass, pipelineLayout, descrSetLayout, descrSets})
        , m_colorAttIndices {}
    { }

    void GraphicsPipeline::destroyImpl()
    {
        vkDestroyPipeline(getDevice(), getPipeline(), nullptr);
        vkDestroyPipelineLayout(getDevice(), getPipelineLayout(), nullptr);
        vkDestroyRenderPass(getDevice(), getRenderPass(), nullptr);
        vkDestroyDescriptorSetLayout(getDevice(), getDescriptorSetLayout(), nullptr);
    }

    GraphicsPipeline* GraphicsPipeline::createImpl(VkDevice device, GraphicsPipelineInfo info)
    {
        auto state = App::current()->getState();

        // ============ Extract parameters ============= //

        auto attribDescrs = info.vertex.attributes;
        auto bindingDescr = info.vertex.binding;
        auto textures     = info.descriptor.textureBindings;
        auto ubos         = info.descriptor.uboBindings;

        // ============== Descriptor Set =============== //

        VkDescriptorSetLayout descrSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descrSets;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        {
            std::transform(
                ubos.begin(), ubos.end(), std::back_inserter(bindings), [](PipelineUniformBufferBinding ubo) {
                    return ubo.getDescriptorSetLayoutBinding();
                });
            std::transform(textures.begin(),
                           textures.end(),
                           std::back_inserter(bindings),
                           [](PipelineTextureBinding tex) { return tex.getDescriptorSetLayoutBinding(); });
            VkDescriptorSetLayoutCreateInfo descrSetLayoutInfo {
                .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings    = bindings.data(),
            };
            if (vkCreateDescriptorSetLayout(state.vk.device, &descrSetLayoutInfo, nullptr, &descrSetLayout)
                != VK_SUCCESS)
            {
                throw std::runtime_error(utils::makeErrorMessage(
                    "VK::CREATE", "Failed to create uniform buffer descriptor set layout"));
            }

            std::vector<VkDescriptorSetLayout> layouts(state.vk.swapChain.images.size(), descrSetLayout);
            VkDescriptorSetAllocateInfo descrAllocInfo {
                .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool     = state.vk.descriptor.pool,
                .descriptorSetCount = static_cast<uint32_t>(state.vk.swapChain.images.size()),
                .pSetLayouts        = layouts.data(),
            };
            descrSets.resize(state.vk.swapChain.images.size());
            if (vkAllocateDescriptorSets(state.vk.device, &descrAllocInfo, descrSets.data()) != VK_SUCCESS)
            {
                throw std::runtime_error(
                    utils::makeErrorMessage("VK::PIPELINE", "Failed to create Vulkan descriptor sets"));
            }

            // Configure descriptors for UBOs
            std::vector<VkWriteDescriptorSet> writes;
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            bufferInfos.reserve(ubos.size() * state.vk.swapChain.images.size());
            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.reserve(textures.size() * state.vk.swapChain.images.size());

            for (uint32_t i = 0; i < state.vk.swapChain.images.size(); ++i)
            {
                for (const auto& ubo : ubos)
                {
                    VkBuffer buffer = ubo.getBuffer();
                    VkDeviceSize sz = ubo.getSize();
                    VkDescriptorBufferInfo bufferInfo {
                        .buffer = buffer,
                        .offset = 0,
                        .range  = sz,
                    };
                    bufferInfos.push_back(bufferInfo);

                    VkWriteDescriptorSet descriptorWrite {
                        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet           = descrSets[i],
                        .dstBinding       = ubo.binding,
                        .dstArrayElement  = 0,
                        .descriptorCount  = 1,
                        .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo       = nullptr,
                        .pBufferInfo      = &bufferInfos[bufferInfos.size() - 1],
                        .pTexelBufferView = nullptr,
                    };

                    writes.push_back(descriptorWrite);
                }

                for (const auto& tex : textures)
                {
                    VkDescriptorImageInfo imageInfo {
                        .sampler     = tex.getSampler(),
                        .imageView   = tex.getImageView(),
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    };
                    imageInfos.push_back(imageInfo);

                    VkWriteDescriptorSet descriptorWrite {
                        .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet           = descrSets[i],
                        .dstBinding       = tex.binding,
                        .dstArrayElement  = 0,
                        .descriptorCount  = 1,
                        .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo       = &imageInfos[imageInfos.size() - 1],
                        .pBufferInfo      = nullptr,
                        .pTexelBufferView = nullptr,
                    };
                    writes.push_back(descriptorWrite);
                }
            }
            vkUpdateDescriptorSets(state.vk.device, writes.size(), writes.data(), 0, nullptr);
        }

        // ###################### Pipeline ####################### //

        VkPipelineLayout pipelineLayout;
        VkRenderPass renderPass;
        VkPipeline graphicsPipeline;

        const auto assetsDir = App::current()->getAssetsDir();

        // ========== Create shader modules =========== //

        using shadermodule_info_t = std::tuple<fs::path, VkShaderStageFlagBits, VkShaderModule>;
        std::vector<shadermodule_info_t> shaderModules;
        shaderModules.reserve(6);

        auto pathToShaderModule = [&assetsDir, &device](const fs::path& p,
                                                        VkShaderStageFlagBits stage) -> shadermodule_info_t {
            auto shaderExt    = p.stem().extension();
            auto shaderP      = assetsDir / p;
            auto shaderCode   = readSPIRVFile(shaderP);
            auto shaderModule = createShaderModule(device, shaderCode, p);
            return {shaderP, stage, shaderModule};
        };
        if (info.shaderPath.vert.has_value())
            shaderModules.push_back(pathToShaderModule(*info.shaderPath.vert, VK_SHADER_STAGE_VERTEX_BIT));
        if (info.shaderPath.frag.has_value())
            shaderModules.push_back(pathToShaderModule(*info.shaderPath.frag, VK_SHADER_STAGE_FRAGMENT_BIT));
        if (info.shaderPath.tese.has_value())
            shaderModules.push_back(
                pathToShaderModule(*info.shaderPath.tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT));
        if (info.shaderPath.tesc.has_value())
            shaderModules.push_back(
                pathToShaderModule(*info.shaderPath.tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT));
        if (info.shaderPath.geom.has_value())
            shaderModules.push_back(pathToShaderModule(*info.shaderPath.geom, VK_SHADER_STAGE_GEOMETRY_BIT));
        if (info.shaderPath.comp.has_value())
            shaderModules.push_back(pathToShaderModule(*info.shaderPath.comp, VK_SHADER_STAGE_COMPUTE_BIT));

        // =========== Create shader stages =========== //

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages(shaderModules.size());
        auto shaderModuleToStage =
            [](const shadermodule_info_t& shaderMod) -> VkPipelineShaderStageCreateInfo {
            return {
                .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage  = std::get<1>(shaderMod),
                .module = std::get<2>(shaderMod),
                .pName  = "main",
            };
        };
        std::transform(shaderModules.begin(), shaderModules.end(), shaderStages.begin(), shaderModuleToStage);

        // ======= Fixed Function Configuration ======= //

        VkPipelineVertexInputStateCreateInfo vertexInputInfo {
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = 1,
            .pVertexBindingDescriptions      = &bindingDescr,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescrs.size()),
            .pVertexAttributeDescriptions    = attribDescrs.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkViewport viewport {
            .x        = 0.0f,
            .y        = 0.0f,
            .width    = static_cast<float>(state.vk.swapChain.extent.width),
            .height   = static_cast<float>(state.vk.swapChain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor {
            .offset = {0, 0},
            .extent = state.vk.swapChain.extent,
        };

        VkPipelineViewportStateCreateInfo viewportState {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports    = &viewport,
            .scissorCount  = 1,
            .pScissors     = &scissor,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = VK_POLYGON_MODE_FILL,
            .cullMode                = VK_CULL_MODE_BACK_BIT,
            .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable         = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp          = 0.0f,
            .depthBiasSlopeFactor    = 0.0f,
            .lineWidth               = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisampling {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable   = VK_FALSE,
            .minSampleShading      = 1.0f,
            .pSampleMask           = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable      = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment {
            .blendEnable         = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                              | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineDepthStencilStateCreateInfo depthStencil {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable       = VK_TRUE,
            .depthWriteEnable      = VK_TRUE,
            .depthCompareOp        = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = VK_FALSE,
        };

        VkPipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        // ============= Pipeline Layout ============== //

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = 0,
            .pSetLayouts            = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
        };
        if (descrSetLayout != VK_NULL_HANDLE)
        {
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts    = &descrSetLayout;
        }

        if (vkCreatePipelineLayout(state.vk.device, &pipelineLayoutInfo, nullptr, &pipelineLayout)
            != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan pipeline layout"));
        }

        // ============ Create Render Pass ============ //

        VkAttachmentDescription colorAttachment {};
        colorAttachment.format         = state.vk.swapChain.format;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment {};
        depthAttachment.format         = state.vk.swapChain.depthImage->getFormat();
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency {};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        std::vector<VkAttachmentDescription> attachments = {
            colorAttachment,
            depthAttachment,
        };
        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        if (vkCreateRenderPass(state.vk.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan render pass"));
        }

        // ============= Create Pipeline ============== //

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = shaderStages.size();
        pipelineInfo.pStages             = shaderStages.data();
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = &depthStencil;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = nullptr;
        pipelineInfo.layout              = pipelineLayout;
        pipelineInfo.renderPass          = renderPass;
        pipelineInfo.subpass             = 0;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex   = -1;

        if (vkCreateGraphicsPipelines(
                state.vk.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)
            != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan graphics pipeline"));
        }

        // ========== Destroy shader modules ========== //

        auto shaderModuleDeleter = [&state](const shadermodule_info_t& shaderMod) {
            vkDestroyShaderModule(state.vk.device, std::get<2>(shaderMod), nullptr);
        };
        std::for_each(shaderModules.begin(), shaderModules.end(), shaderModuleDeleter);

        // ====== Create/Return pipeline wrapper ====== //

        auto* pipeline = new GraphicsPipeline(
            state.vk.device, graphicsPipeline, renderPass, pipelineLayout, descrSetLayout, descrSets);

        // Set pipline attachment indices
        pipeline->m_colorAttIndices = {0};

        return pipeline;
    }

} // namespace ivulk
