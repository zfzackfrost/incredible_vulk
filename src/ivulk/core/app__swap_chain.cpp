#include <ivulk/core/app.hpp>
#include <ivulk/utils/messages.hpp>

#include <SDL2/SDL_vulkan.h>

#include <algorithm>

namespace ivulk {

	void App::createVkSurface()
	{
		if (SDL_Vulkan_CreateSurface(state.sdl.window, state.vk.instance, &state.vk.surface) != SDL_TRUE)
		{
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CREATE", "Failed to create the Vulkan surface"));
		}
	}

	VkSurfaceFormatKHR App::chooseVkSwapFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats)
	{
		auto idealMatcher = [](VkSurfaceFormatKHR fmt) -> bool {
			return fmt.format == VK_FORMAT_B8G8R8A8_SRGB
				&& fmt.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		};
		auto ideal = std::find_if(supportedFormats.begin(), supportedFormats.end(), idealMatcher);
		if (ideal != supportedFormats.end())
			return *ideal;
		return supportedFormats.front();
	}
	VkPresentModeKHR App::chooseVkPresentMode(const std::vector<VkPresentModeKHR>& supportedModes)
	{
		auto idealMatcher = [](VkPresentModeKHR mode) -> bool { return mode == VK_PRESENT_MODE_IMMEDIATE_KHR; };
		auto ideal = std::find_if(supportedModes.begin(), supportedModes.end(), idealMatcher);
		if (ideal != supportedModes.end())
			return *ideal;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D App::chooseVkSwapExtent(const VkSurfaceCapabilitiesKHR& capabilties)
	{
		if (capabilties.currentExtent.width != UINT32_MAX)
		{
			return capabilties.currentExtent;
		}
		else
		{
			int w,h;
			SDL_Vulkan_GetDrawableSize(state.sdl.window, &w, &h);

			auto clamp_ui = [](uint32_t x, uint32_t mn, uint32_t mx) {
				return (x < mn) ? (mn) : (x > mx ? mx : x);
			};
			VkExtent2D actualExtent {static_cast<uint32_t>(w),
									 static_cast<uint32_t>(h)};
			actualExtent.width = clamp_ui(actualExtent.width, capabilties.minImageExtent.width,
										  capabilties.maxImageExtent.width);
			actualExtent.height = clamp_ui(actualExtent.height, capabilties.minImageExtent.height,
										   capabilties.maxImageExtent.height);
			return actualExtent;
		}
	}

	SwapChainInfo App::querySwapChainInfo(VkPhysicalDevice device)
	{
		SwapChainInfo scInfo;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, state.vk.surface, &scInfo.capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, state.vk.surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			scInfo.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, state.vk.surface, &formatCount,
												 scInfo.formats.data());
		}

		uint32_t presentModesCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, state.vk.surface, &presentModesCount, nullptr);
		if (presentModesCount != 0)
		{
			scInfo.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, state.vk.surface, &presentModesCount,
													  scInfo.presentModes.data());
		}

		return scInfo;
	}

	void App::createVkImageViews()
	{
		auto& images = state.vk.swapChain.images;
		auto& imageViews = state.vk.swapChain.imageViews;
		imageViews.resize(images.size());

		for (std::size_t i = 0; i < imageViews.size(); ++i)
		{
			VkImageViewCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = state.vk.swapChain.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
			if (vkCreateImageView(state.vk.device, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error(utils::makeErrorMessage(
					"VK::CREATE", "Failed to create Vulkan image view for a swap chain image"));
			}
		}

		if (getPrintDbg())
		{
			std::cout << utils::makeSuccessMessage("VK::CREATE", "Created the Vulkan swap chain image views")
					  << std::endl;
		}
	}

	void App::createVkSwapChain()
	{
		SwapChainInfo scInfo = querySwapChainInfo(state.vk.physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseVkSwapFormat(scInfo.formats);
		VkPresentModeKHR presentMode = chooseVkPresentMode(scInfo.presentModes);
		VkExtent2D extent = chooseVkSwapExtent(scInfo.capabilities);

		uint32_t imageCount = scInfo.capabilities.minImageCount + 1;
		if (scInfo.capabilities.maxImageCount > 0 && imageCount > scInfo.capabilities.maxImageCount)
		{
			imageCount = scInfo.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = state.vk.surface,
			.minImageCount = imageCount,
			.imageFormat = surfaceFormat.format,
			.imageColorSpace = surfaceFormat.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = scInfo.capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE,
		};

		QueueFamilyIndices indices = findVkQueueFamilies(state.vk.physicalDevice);
		uint32_t queueFamilies[] = {indices.graphics.value(), indices.present.value()};
		if (indices.graphics != indices.present)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilies;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		if (vkCreateSwapchainKHR(state.vk.device, &createInfo, nullptr, &state.vk.swapChain.sc) != VK_SUCCESS)
		{
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan swap chain"));
		}
		else if (getPrintDbg())
		{
			std::cout << utils::makeSuccessMessage("VK::CREATE", "Created the Vulkan swap chain")
					  << std::endl;
		}

		// Get swap chain images
		vkGetSwapchainImagesKHR(state.vk.device, state.vk.swapChain.sc, &imageCount, nullptr);
		state.vk.swapChain.images.resize(imageCount);
		vkGetSwapchainImagesKHR(state.vk.device, state.vk.swapChain.sc, &imageCount,
								state.vk.swapChain.images.data());

		state.vk.swapChain.format = std::move(surfaceFormat.format);
		state.vk.swapChain.extent = std::move(extent);
	}

	void App::createVkFramebuffers()
	{

		auto& pipelinePtr = state.vk.pipelines.mainGfx;
		if (auto pipeline = pipelinePtr.lock())
		{
			auto& swapChain = state.vk.swapChain;
			swapChain.framebuffers.resize(swapChain.imageViews.size());
			auto& extent = swapChain.extent;
			for (std::size_t i = 0; i < swapChain.imageViews.size(); ++i)
			{
				VkImageView attachments[] = {swapChain.imageViews[i]};
				VkFramebufferCreateInfo createInfo {
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.renderPass = pipeline->getRenderPass(),
					.attachmentCount = 1,
					.pAttachments = attachments,
					.width = extent.width,
					.height = extent.height,
					.layers = 1,
				};
				if (vkCreateFramebuffer(state.vk.device, &createInfo, nullptr, &swapChain.framebuffers[i])
					!= VK_SUCCESS)
				{
					throw std::runtime_error(
						utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan framebuffers"));
				}
			}
			if (getPrintDbg())
			{
				std::cout << utils::makeSuccessMessage("VK::CREATE", "Created the Vulkan framebuffers")
						  << std::endl;
			}
		}
		else
		{
			throw std::runtime_error(utils::makeErrorMessage(
				"VK::STATE", "Failed to create Vulkan framebuffers: No main graphics pipeline set"));
		}
	}

	void App::cleanupVkSwapChain()
	{
		cleanup(true);
		for (auto framebuffer : state.vk.swapChain.framebuffers)
		{
			vkDestroyFramebuffer(state.vk.device, framebuffer, nullptr);
		}
		for (const auto& imgV : state.vk.swapChain.imageViews)
			vkDestroyImageView(state.vk.device, imgV, nullptr);
		vkDestroySwapchainKHR(state.vk.device, state.vk.swapChain.sc, nullptr);

		vkDestroyDescriptorPool(state.vk.device, state.vk.descriptor.pool, nullptr);
	}

	void App::recreateVkSwapChain()
	{
		vkDeviceWaitIdle(state.vk.device);

		cleanupVkSwapChain();

		createVkSwapChain();
		createVkImageViews();
		createVkDescriptorPool();
		// Run subclass initialization before creating framebuffers
		initialize(true);

		createVkFramebuffers();
	}


} // namespace ivulk
