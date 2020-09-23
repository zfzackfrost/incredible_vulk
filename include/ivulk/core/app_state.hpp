/**
 * @file app_state.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `AppState` structure.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/framebuffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/queue_families.hpp>
#include <ivulk/core/vma.hpp>

#include <SDL2/SDL.h>
#include <ivulk/vk.hpp>

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
		 * @brief Vulkan application state
		 */
        struct
        {
            vk::Instance instance {nullptr};             ///< The Vulkan instance handle
            vk::Device device {nullptr};                 ///< The Vulkan logical device handle
            vk::PhysicalDevice physicalDevice {nullptr}; ///< The Vulkan physical device handle

            VkSurfaceKHR surface                    = VK_NULL_HANDLE; ///< The Vulkan surface handle
            VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE; ///< The Vulkan debug messenger handle
            std::vector<const char*> requiredLayers; ///< The Vulkan layers required for creating the instance
            VmaAllocator allocator = VK_NULL_HANDLE; ///< The VMA (Vulkan Memory Allocator) allocator handle

            /**
             * @brief Handles and state for Vulkan syncronization
             */
            struct
            {
                std::vector<VkSemaphore>
                    imageAvailableSems; ///< Vulkan semaphores for when a swapchain image is available
                std::vector<VkSemaphore>
                    renderFinishedSems;              ///< Vullkan semephores for when rendering is finshed
                std::vector<VkFence> inFlightFences; ///< Vulkan fences for concurrent frames
                std::vector<VkFence> imagesInFlight; ///< Vulkan fences for cuncurrent swapchain images
            } sync;

            /**
             * @brief Vulkan pipeline handles
             */
            struct
            {
                std::weak_ptr<GraphicsPipeline>
                    mainGfx; ///< GraphicsPipeline that is used for rendering the final image to the swapchain
            } pipelines;

            /**
             * @brief Handles and state for Vulkan descriptor pools, sets, etc.
             */
            struct
            {
                VkDescriptorPool pool; ///< Primary Vulkan descriptor pool
            } descriptor;

            /**
             * @brief Handles and state for Vulkan queues
             */
            struct
            {
                vk::Queue graphics {nullptr}; ///< Vulkan queue handle that supports graphics operations
                vk::Queue present {nullptr};  ///< Vulkan queue handle that supports present operations
            } queues;

            /**
			 * @brief Handles and state for Vulkan swapchain
			 */
            struct
            {
                VkSwapchainKHR sc = VK_NULL_HANDLE;         ///< Vulkan swapchain handle
                std::vector<VkImage> images;                ///< Vulkan swapchain image handles
                std::vector<VkImageView> imageViews;        ///< Vulkan swapchain image view handles
                std::vector<Framebuffer::Ptr> framebuffers; ///< Vulkan swapchain framebuffer handles
                VkFormat format;                            ///< Vulkan swapchain format
                VkExtent2D extent;                          ///< Vulkan swapchain extent
                Image::Ptr depthImage;                      ///< Vulkan image for swapchain depth component
                uint32_t maxFramesInFlight = 2;             ///< Maximum frames number to enqueue
            } swapChain;

            /**
             * @brief Handles and state for Vulkan command buffers, pools, etc.
             */
            struct
            {
                VkCommandPool gfxPool;                   ///< Vulkan command pool for graphics operations
                std::vector<VkCommandBuffer> gfxBuffers; ///< Vulkan command buffers for graphics operations
                std::shared_ptr<CommandBuffers> renderCmdBufs; /// < Vulkan command buffers for rendering
            } cmd;
        } vk;
    };
} // namespace ivulk
