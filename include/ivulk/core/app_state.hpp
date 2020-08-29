/**
 * @file app_state.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `AppState` structure.
 */

#pragma once

#include <ivulk/core/queue_families.hpp>
#include <ivulk/core/graphics_pipeline.hpp>

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <string>

namespace ivulk {
	struct AppState
	{
		std::vector<std::string> cmdArgs;
	
		/**
		 * @brief SDL2 application state
		 */
		struct
		{
			SDL_Window* window = nullptr; ///< Main application window
		} sdl;

		/**
		 * @brief Events application state
		 */
		struct
		{
			bool shouldQuit = false;
		} evt;

		/**
		 * @brief Application vulkan state
		 */
		struct
		{
			VkInstance instance = VK_NULL_HANDLE; ///< The Vulkan instance
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
			VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;
			std::vector<const char*> requiredLayers;

			struct
			{
				std::vector<VkSemaphore> imageAvailableSems;
				std::vector<VkSemaphore> renderFinishedSems;
				std::vector<VkFence> inFlightFences;
				std::vector<VkFence> imagesInFlight;
			} sync;
			
			struct
			{
				std::weak_ptr<GraphicsPipeline> mainGfx;
			} pipelines;

			struct
			{
				VkQueue graphics = VK_NULL_HANDLE;
				VkQueue present = VK_NULL_HANDLE;
			} queues;

			/**
			 * @brief Vulkan swap chain and related
			 */
			struct
			{
				VkSwapchainKHR sc = VK_NULL_HANDLE;
				std::vector<VkImage> images;
				std::vector<VkImageView> imageViews;
				std::vector<VkFramebuffer> framebuffers;
				VkFormat format;
				VkExtent2D extent;
			} swapChain;

			struct
			{
				VkCommandPool gfxPool;
				std::vector<VkCommandBuffer> gfxBuffers;
			} cmd;
		} vk;
	};
} // namespace ivulk
