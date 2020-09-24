#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/image.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/utils/commands.hpp>
#include <ivulk/utils/format.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ivulk {
    namespace fs = boost::filesystem;
    Image::Image(VkDevice device, VkImage image, VmaAllocation allocation, VkImageView view)
        : base_t(device, handles_t {image, allocation, view})
        , m_mipLevels(1u)
    { }

    void transitionImageLayout(
        VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
    {
        VkCommandBuffer commandBuffer = utils::beginOneTimeCommands();

        VkImageMemoryBarrier barrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (utils::hasStencilComponent(format))
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                 && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
                 && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                                    | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else
        {
            throw std::invalid_argument(
                utils::makeErrorMessage("VK::IMAGE", "Unsupported layout transition"));
        }

        // clang-format off
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
        // clang-format on

        utils::endOneTimeCommands(commandBuffer);
    }
    void makeImage(VkImage& outImage,
                   VmaAllocation& outAlloc,
                   ImageInfo createInfo,
                   VkExtent3D extent,
                   VkFormat format,
                   uint32_t mipLevels)
    {
        VkImageUsageFlags usage = createInfo.usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usage |= (createInfo.load.bEnable && createInfo.load.bGenMips) ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0u;
        const VkImageCreateInfo imageInfo {
            .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext       = nullptr,
            .flags       = 0,
            .imageType   = VK_IMAGE_TYPE_2D,
            .format      = format,
            .extent      = extent,
            .mipLevels   = mipLevels,
            .arrayLayers = 1,
            .samples     = VK_SAMPLE_COUNT_1_BIT,
            .tiling      = createInfo.tiling,
            .usage       = usage,
            .sharingMode = createInfo.sharingMode,
        };

        auto allocator = App::current()->getState().vk.allocator;
        VmaAllocationCreateInfo allocInfo {
            .usage = createInfo.memoryMode,
        };
        VmaAllocation alloc = VK_NULL_HANDLE;
        if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &outImage, &outAlloc, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan image"));
        }
    }

    void generateMipMaps(vk::Image image, vk::Extent3D extent, uint32_t mipLevels)
    {
        vk::CommandBuffer cmdBuf(utils::beginOneTimeCommands());

        vk::ImageMemoryBarrier barrier {};
        barrier.image                           = image;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0u;
        barrier.subresourceRange.layerCount     = 1u;
        barrier.subresourceRange.levelCount     = 1u;

        uint32_t mipWidth  = extent.width;
        uint32_t mipHeight = extent.height;

        for (auto i = 1u; i < mipLevels; ++i)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout                     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask                 = vk::AccessFlagBits::eTransferRead;

            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   static_cast<vk::DependencyFlags>(0),
                                   0,
                                   nullptr,
                                   0,
                                   nullptr,
                                   1,
                                   &barrier);
            vk::ImageBlit blit {};
            blit.srcOffsets[0]                 = vk::Offset3D(0, 0, 0);
            blit.srcOffsets[1]                 = vk::Offset3D(mipWidth, mipHeight, 1);
            blit.srcSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = 0u;
            blit.srcSubresource.layerCount     = 1u;
            blit.dstOffsets[0]                 = vk::Offset3D(0, 0, 0);
            blit.dstOffsets[1]                 = vk::Offset3D(
                mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
            blit.dstSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = 0u;
            blit.dstSubresource.layerCount     = 1u;

            cmdBuf.blitImage(image,
                             vk::ImageLayout::eTransferSrcOptimal,
                             image,
                             vk::ImageLayout::eTransferDstOptimal,
                             1,
                             &blit,
                             vk::Filter::eLinear);

            barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   static_cast<vk::DependencyFlags>(0),
                                   0,
                                   nullptr,
                                   0,
                                   nullptr,
                                   1,
                                   &barrier);

            if (mipWidth > 1)
                mipWidth /= 2;
            if (mipHeight > 1)
                mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout                     = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout                     = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask                 = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask                 = vk::AccessFlagBits::eShaderRead;

        cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                               vk::PipelineStageFlagBits::eFragmentShader,
                               static_cast<vk::DependencyFlags>(0),
                               0,
                               nullptr,
                               0,
                               nullptr,
                               1,
                               &barrier);

        utils::endOneTimeCommands(cmdBuf);
    }

    Image* Image::createImpl(VkDevice device, ImageInfo createInfo)
    {
        auto physDevice = App::current()->getState().vk.physicalDevice;


        VkExtent3D extent = createInfo.extent;
        VkImage image     = VK_NULL_HANDLE;
        VkImageView view  = VK_NULL_HANDLE;
        VmaAllocation alloc;
        VkFormat format    = createInfo.format;
        uint32_t mipLevels = 1u;
        if (createInfo.load.bEnable)
        {
            auto p = createInfo.load.path;
            if (p.is_relative())
            {
                p = App::current()->getAssetsDir() / p;
            }
            auto pStr = p.string();
            int texW, texH, texCh;
            void* pixels = nullptr;
            VkDeviceSize imageSize;
            if (createInfo.load.bHDR)
            {
                pixels        = stbi_loadf(pStr.c_str(), &texW, &texH, &texCh, STBI_rgb_alpha);
                imageSize = texW * texH * sizeof(float) * 4;
            }
            else
            {
                pixels        = stbi_load(pStr.c_str(), &texW, &texH, &texCh, STBI_rgb_alpha);
                imageSize = texW * texH * 4;
            }
            if (!pixels)
            {
                throw std::runtime_error(utils::makeErrorMessage("VK::TEX", "Failed to load texture"));
            }

            Buffer::Ptr stagingBuffer = Buffer::create(device,
                                                       {
                                                           .size       = imageSize,
                                                           .usage      = E_BufferUsage::TransferSrc,
                                                           .memoryMode = E_MemoryMode::CpuToGpu,
                                                       });
            stagingBuffer->fillBuffer(pixels, imageSize);

            extent = {
                .width  = static_cast<uint32_t>(texW),
                .height = static_cast<uint32_t>(texH),
                .depth  = 1,
            };

            stbi_image_free(pixels);

            if (createInfo.load.bGenMips)
                mipLevels = calcMipLevels(extent);
            
            if (createInfo.load.bHDR)
            {
                format = VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            else
                format = (createInfo.load.bSrgb) ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            makeImage(image, alloc, createInfo, extent, format, mipLevels);

            transitionImageLayout(
                image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
            utils::copyBufferToImage(
                stagingBuffer->getBuffer(), image, static_cast<uint32_t>(texW), static_cast<uint32_t>(texH));
            if (createInfo.load.bGenMips) { 
                generateMipMaps(image, extent, mipLevels);
            }
            else
            {
                transitionImageLayout(
                    image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, createInfo.layout, mipLevels);
            }
        }
        else
        {
            makeImage(image, alloc, createInfo, extent, createInfo.format, mipLevels);
        }
        
        VkImageViewCreateInfo viewInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = {
				.aspectMask = createInfo.aspect,
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
        if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to make Vulkan image view"));
        }

        // Create/return new `Image*`
        auto ret      = new Image(device, image, alloc, view);
        ret->m_format = format;
        ret->m_extent = extent;
        return ret;
    }

    void Image::destroyImpl()
    {
        auto allocator = App::current()->getState().vk.allocator;
        vkDestroyImageView(getDevice(), getImageView(), nullptr);
        vmaDestroyImage(allocator, getImage(), getAllocation());
    }

    uint32_t Image::calcMipLevels(const VkExtent3D extent)
    {
        return static_cast<uint32_t>(
                   glm::floor(glm::log2(static_cast<double>(glm::max(extent.width, extent.height)))))
               + 1u;
    }
} // namespace ivulk
