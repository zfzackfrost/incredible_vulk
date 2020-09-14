/**
 * @file app_state.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `AppState` structure.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/queue_families.hpp>
#include <ivulk/core/vma.hpp>
#include <ivulk/core/framebuffer.hpp>

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

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
            VkInstance instance                     = VK_NULL_HANDLE; ///< The Vulkan instance
            VkSurfaceKHR surface                    = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice         = VK_NULL_HANDLE;
            VkDevice device                         = VK_NULL_HANDLE;
            std::vector<const char*> requiredLayers;
            VmaAllocator allocator = VK_NULL_HANDLE;

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
                VkDescriptorPool pool;
            } descriptor;

            struct
            {
                VkQueue graphics = VK_NULL_HANDLE;
                VkQueue present  = VK_NULL_HANDLE;
            } queues;

            /**
			 * @brief Vulkan swap chain and related
			 */
            struct
            {
                VkSwapchainKHR sc = VK_NULL_HANDLE;
                std::vector<VkImage> images;
                std::vector<VkImageView> imageViews;
                std::vector<Framebuffer::Ptr> framebuffers;
                VkFormat format;
                VkExtent2D extent;
                Image::Ptr depthImage;
            } swapChain;

            struct
            {
                VkCommandPool gfxPool;
                std::vector<VkCommandBuffer> gfxBuffers;
                std::shared_ptr<CommandBuffers> renderCmdBufs;
            } cmd;
        } vk;
    };
} // namespace ivulk
