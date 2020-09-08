#include <ivulk/core/image.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/utils/commands.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ivulk {
	namespace fs = boost::filesystem;
	Image::Image(VkDevice device, VkImage image, VmaAllocation allocation, VkImageView view)
		: base_t(device, handles_t {image, allocation, view})
	{ }

	void
	transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
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
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

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
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
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
	void makeImage(
		VkImage& outImage, VmaAllocation& outAlloc, ImageInfo createInfo, VkExtent3D extent, VkFormat format)
	{
		VkImageUsageFlags usage = createInfo.usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		const VkImageCreateInfo imageInfo {
			.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext       = nullptr,
			.flags       = 0,
			.imageType   = VK_IMAGE_TYPE_2D,
			.format      = format,
			.extent      = extent,
			.mipLevels   = 1,
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

	Image* Image::createImpl(VkDevice device, ImageInfo createInfo)
	{
		VkExtent3D extent = createInfo.extent;
		VkImage image     = VK_NULL_HANDLE;
		VkImageView view  = VK_NULL_HANDLE;
		VmaAllocation alloc;
		VkFormat format = createInfo.format;
		if (createInfo.load.bEnable)
		{
			auto p = createInfo.load.path;
			if (p.is_relative())
			{
				p = App::current()->getAssetsDir() / p;
			}
			auto pStr = p.string();
			int texW, texH, texCh;
			stbi_uc* pixels        = stbi_load(pStr.c_str(), &texW, &texH, &texCh, STBI_rgb_alpha);
			VkDeviceSize imageSize = texW * texH * 4;
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

			format = (createInfo.load.bSrgb) ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UINT;
			makeImage(image, alloc, createInfo, extent, format);

			transitionImageLayout(
				image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			utils::copyBufferToImage(
				stagingBuffer->getBuffer(), image, static_cast<uint32_t>(texW), static_cast<uint32_t>(texH));
			transitionImageLayout(
				image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, createInfo.layout);
		}
		else
		{
			makeImage(image, alloc, createInfo, extent, createInfo.format);
		}

		VkImageViewCreateInfo viewInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
		{
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CREATE", "Failed to make Vulkan image view"));
		}

		return new Image(device, image, alloc, view);
	}

	void Image::destroyImpl()
	{
		auto allocator = App::current()->getState().vk.allocator;
		vkDestroyImageView(getDevice(), getImageView(), nullptr);
		vmaDestroyImage(allocator, getImage(), getAllocation());
	}
} // namespace ivulk