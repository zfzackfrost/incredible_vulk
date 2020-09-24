#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/render/standard_shader.hpp>

#include <ivulk/core/app.hpp>

#include <array>

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

    vk::ShaderModule GraphicsPipeline::createShaderModule(VkDevice _device,
                                                          const std::vector<char>& shaderCode,
                                                          const fs::path& assetPath)
    {
        vk::Device device(_device);
        vk::ShaderModuleCreateInfo createInfo {};
        createInfo.setCodeSize(shaderCode.size())
            .setPCode(reinterpret_cast<const uint32_t*>(shaderCode.data()));
        auto shaderModule = device.createShaderModule(createInfo);
        if (shaderModule.result != vk::Result::eSuccess)
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
        return shaderModule.value;
    }

    GraphicsPipeline::GraphicsPipeline(vk::Device device,
                                       vk::Pipeline pipeline,
                                       vk::RenderPass renderPass,
                                       vk::PipelineLayout pipelineLayout,
                                       vk::DescriptorSetLayout descrSetLayout,
                                       std::vector<vk::DescriptorSet> descrSets)
        : base_t(device, handles_t {pipeline, renderPass, pipelineLayout, descrSetLayout, descrSets})
        , m_colorAttIndices {}
    { }

    void GraphicsPipeline::destroyImpl()
    {
        vk::Device device(getDevice());
        device.destroy(getPipeline());
        device.destroy(getPipelineLayout());
        device.destroy(getRenderPass());
        device.destroy(getDescriptorSetLayout());
    }

    GraphicsPipeline* GraphicsPipeline::createImpl(VkDevice _device, GraphicsPipelineInfo info)
    {
        vk::Device device(_device);
        auto state = App::current()->getState();

        // ============ Extract parameters ============= //

        auto attribDescrs = info.vertex.attributes;
        auto bindingDescr = info.vertex.binding;
        auto textures     = info.descriptor.textureBindings;
        auto ubos         = info.descriptor.uboBindings;

        // ============== Descriptor Set =============== //

        vk::DescriptorSetLayout descrSetLayout;
        std::vector<vk::DescriptorSet> descrSets;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        {
            std::transform(
                ubos.begin(), ubos.end(), std::back_inserter(bindings), [](PipelineUniformBufferBinding ubo) {
                    return ubo.getDescriptorSetLayoutBinding();
                });
            std::transform(textures.begin(),
                           textures.end(),
                           std::back_inserter(bindings),
                           [](PipelineTextureBinding tex) { return tex.getDescriptorSetLayoutBinding(); });
            vk::DescriptorSetLayoutCreateInfo descrSetLayoutInfo {};
            descrSetLayoutInfo.setBindingCount(bindings.size()).setPBindings(bindings.data());
            auto _descrL = device.createDescriptorSetLayout(descrSetLayoutInfo);
            if (_descrL.result != vk::Result::eSuccess)
            {
                throw std::runtime_error(utils::makeErrorMessage(
                    "VK::CREATE", "Failed to create uniform buffer descriptor set layout"));
            }
            descrSetLayout = _descrL.value;

            std::vector<vk::DescriptorSetLayout> layouts(state.vk.swapChain.images.size(), descrSetLayout);
            vk::DescriptorSetAllocateInfo descrAllocInfo {};
            descrAllocInfo.setDescriptorPool(state.vk.descriptor.pool)
                .setDescriptorSetCount(state.vk.swapChain.images.size())
                .setPSetLayouts(layouts.data());
            auto _descrSets = device.allocateDescriptorSets(descrAllocInfo);
            if (_descrSets.result != vk::Result::eSuccess)
            {
                throw std::runtime_error(
                    utils::makeErrorMessage("VK::PIPELINE", "Failed to create Vulkan descriptor sets"));
            }
            descrSets = _descrSets.value;

            // Configure descriptors for UBOs
            std::vector<vk::WriteDescriptorSet> writes;
            std::vector<vk::DescriptorBufferInfo> bufferInfos;
            bufferInfos.reserve(ubos.size() * state.vk.swapChain.images.size());
            std::vector<vk::DescriptorImageInfo> imageInfos;
            imageInfos.reserve(textures.size() * state.vk.swapChain.images.size());

            for (uint32_t i = 0; i < state.vk.swapChain.images.size(); ++i)
            {
                for (const auto& ubo : ubos)
                {
                    vk::Buffer buffer = ubo.getBuffer();
                    vk::DeviceSize sz = ubo.getSize();
                    vk::DescriptorBufferInfo bufferInfo {};
                    bufferInfo.setBuffer(buffer).setOffset(0u).setRange(sz);
                    bufferInfos.push_back(bufferInfo);

                    vk::WriteDescriptorSet descriptorWrite {};
                    descriptorWrite.setDstSet(descrSets[i])
                        .setDstBinding(ubo.binding)
                        .setDstArrayElement(0u)
                        .setDescriptorCount(1u)
                        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                        .setPImageInfo(nullptr)
                        .setPBufferInfo(&bufferInfos[bufferInfos.size() - 1])
                        .setPTexelBufferView(nullptr);

                    writes.push_back(descriptorWrite);
                }

                for (const auto& tex : textures)
                {
                    vk::DescriptorImageInfo imageInfo {};
                    imageInfo.setSampler(tex.getSampler())
                        .setImageView(tex.getImageView())
                        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
                    imageInfos.push_back(imageInfo);

                    vk::WriteDescriptorSet descriptorWrite {};
                    descriptorWrite.setDstSet(descrSets[i])
                        .setDstBinding(tex.binding)
                        .setDstArrayElement(0u)
                        .setDescriptorCount(1u)
                        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                        .setPImageInfo(&imageInfos[imageInfos.size() - 1])
                        .setPBufferInfo(nullptr)
                        .setPTexelBufferView(nullptr);
                    writes.push_back(descriptorWrite);
                }
            }
            device.updateDescriptorSets(writes, {});
        }

        // ###################### Pipeline ####################### //

        vk::PipelineLayout pipelineLayout;
        vk::RenderPass renderPass;
        vk::Pipeline graphicsPipeline;

        const auto assetsDir = App::current()->getAssetsDir();

        // ========== Create shader modules =========== //

        using shadermodule_info_t = std::tuple<fs::path, VkShaderStageFlagBits, vk::ShaderModule>;
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

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(shaderModules.size());
        auto shaderModuleToStage =
            [](const shadermodule_info_t& shaderMod) -> vk::PipelineShaderStageCreateInfo {
            vk::PipelineShaderStageCreateInfo res {};
            res.setStage(vk::ShaderStageFlagBits(std::get<1>(shaderMod)))
                .setModule(std::get<2>(shaderMod))
                .setPName("main");
            return res;
        };
        std::transform(shaderModules.begin(), shaderModules.end(), shaderStages.begin(), shaderModuleToStage);

        // ======= Fixed Function Configuration ======= //

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo {};
        vertexInputInfo.setVertexBindingDescriptionCount(1u)
            .setPVertexBindingDescriptions(&bindingDescr)
            .setVertexAttributeDescriptionCount(attribDescrs.size())
            .setPVertexAttributeDescriptions(attribDescrs.data());

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly {};
        inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(false);

        vk::Viewport viewport {};
        viewport.setX(0.0f)
            .setY(0.0f)
            .setWidth(static_cast<float>(state.vk.swapChain.extent.width))
            .setHeight(static_cast<float>(state.vk.swapChain.extent.height))
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

        vk::Rect2D scissor {};
        scissor.setOffset(vk::Offset2D(0, 0)).setExtent(vk::Extent2D(state.vk.swapChain.extent));

        vk::PipelineViewportStateCreateInfo viewportState {};
        viewportState.setViewportCount(1u).setPViewports(&viewport).setScissorCount(1u).setPScissors(
            &scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.setDepthClampEnable(false)
            .setRasterizerDiscardEnable(false)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(info.bCullFront ? vk::CullModeFlagBits::eFront : vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(false)
            .setDepthBiasConstantFactor(0.0f)
            .setDepthBiasClamp(0.0f)
            .setDepthBiasSlopeFactor(0.0f)
            .setLineWidth(1.0f);

        vk::PipelineMultisampleStateCreateInfo multisampling {};
        multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(false)
            .setMinSampleShading(1.0f)
            .setPSampleMask(nullptr)
            .setAlphaToCoverageEnable(false)
            .setAlphaToOneEnable(false);

        vk::PipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.setBlendEnable(false)
            .setSrcColorBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eZero)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                               | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.setLogicOpEnable(false)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1u)
            .setPAttachments(&colorBlendAttachment)
            .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

        vk::PipelineDepthStencilStateCreateInfo depthStencil {};
        depthStencil.setDepthTestEnable(info.bDepthEnable)
            .setDepthWriteEnable(info.bDepthEnable)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthBoundsTestEnable(false)
            .setStencilTestEnable(false);

        // ============= Pipeline Layout ============== //

        std::array<vk::PushConstantRange, 1> pushConstantRanges = {{}};

        pushConstantRanges[0]
            .setStageFlags(vk::ShaderStageFlags(E_ShaderStage::All))
            .setOffset(0u)
            .setSize(sizeof(MatricesPushConstants));

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.setSetLayoutCount(0u)
            .setPSetLayouts(nullptr)
            .setPushConstantRangeCount(pushConstantRanges.size())
            .setPPushConstantRanges(pushConstantRanges.data());
        if (descrSetLayout)
        {
            pipelineLayoutInfo.setSetLayoutCount(1u).setPSetLayouts(&descrSetLayout);
        }

        auto _plLayout = device.createPipelineLayout(pipelineLayoutInfo);
        if (_plLayout.result != vk::Result::eSuccess)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan pipeline layout"));
        }
        pipelineLayout = _plLayout.value;

        // ============ Create Render Pass ============ //

        vk::AttachmentDescription colorAttachment {};
        colorAttachment.format         = static_cast<vk::Format>(state.vk.swapChain.format);
        colorAttachment.samples        = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentDescription depthAttachment {};
        depthAttachment.format         = static_cast<vk::Format>(state.vk.swapChain.depthImage->getFormat());
        depthAttachment.samples        = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp        = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference depthAttachmentRef {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass {};
        subpass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        vk::SubpassDependency dependency {};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = static_cast<vk::AccessFlagBits>(0);
        dependency.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        std::vector<vk::AttachmentDescription> attachments = {
            colorAttachment,
            depthAttachment,
        };
        vk::RenderPassCreateInfo renderPassInfo {};
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        auto _renderPass = device.createRenderPass(renderPassInfo);
        // if (vkCreateRenderPass(state.vk.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        if (_renderPass.result != vk::Result::eSuccess)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan render pass"));
        }
        renderPass = _renderPass.value;

        // ============= Create Pipeline ============== //

        vk::GraphicsPipelineCreateInfo pipelineInfo {};
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
        pipelineInfo.basePipelineHandle  = nullptr;
        pipelineInfo.basePipelineIndex   = -1;

        auto _pl = device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline);
        if (_pl != vk::Result::eSuccess)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan graphics pipeline"));
        }

        // ========== Destroy shader modules ========== //

        auto shaderModuleDeleter = [&device](const shadermodule_info_t& shaderMod) {
            device.destroy(std::get<2>(shaderMod));
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
