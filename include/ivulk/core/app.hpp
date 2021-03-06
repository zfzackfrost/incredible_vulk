/**
 * @file app.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `App` class and related.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/app_state.hpp>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/event.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/swap_chain.hpp>
#include <ivulk/core/texture.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/core/vertex.hpp>
#include <ivulk/utils/version_data.hpp>

#include <boost/filesystem.hpp>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace ivulk {

    class UniformBufferObject;

    /**
     * @brief Application base class.
     *
     * Handles many important tasks required by all Incredible Vulk applications.
     */
    class App
    {
    public:
        /**
         * @brief Default constructor
         */
        App(int argc, char* argv[]); // NOLINT

        /**
         * @brief Virtual destructor
         */
        virtual ~App() = default;

        /**
         * @brief Run the application.
         */
        void run();

        /**
         * @brief Static access to the the currently running application
         */
        static App* current();

        /**
         * @brief Get the current app state.
         */
        [[nodiscard]] AppState getState() const;

        /**
         * @brief Helper method to check if debug printing was enabled in the init args.
         */
        [[nodiscard]] bool getPrintDbg() const;

        /**
         * @brief Get the path to the assets directory
         *
         * Pure Virtual
         */
        virtual boost::filesystem::path getAssetsDir() = 0;

        /**
		 * @brief Schedule the app to quit at the end of this frame
		 */
        void quit();

    protected:
        /**
         * @brief Information used when initializing the app
         */
        struct InitArgs
        {
            std::string appName           = "";
            utils::VersionData appVersion = {0, 1, 0};
            bool bDebugPrint              = false;
            struct
            {
                int width        = -1;
                int height       = -1;
                bool bResizable  = false;
                bool bBorderless = false;
                bool bFullscreen = false;
            } window;
            struct
            {
                bool bEnableValidation        = false;
                std::size_t maxFramesInFlight = 2;
            } vk;
        };

        /**
         * @brief Perform subclass-specific initialization.
         *
         * @param swapchainOnly Skip initializing anything that doesn't depend on the swapchain.
         *                      This is `false` for first-time initialization, and `true` when
         *                      recreating the swapchain.
         *
         * Pure virtual.
         */
        virtual void initialize(bool swapchainOnly = false) = 0;

        /**
         * @brief Perform subclass-specific cleanup.
         *
         * @param swapchainOnly Skip cleanup of anything that doesn't depend on the swapchain.
         *                      This is `false` for main application cleanup, and `true` when
         *                      recreating the swapchain.
         * Pure virtual.
         */
        virtual void cleanup(bool swapchainOnly) = 0;

        /**
         * @brief Perform rendering operations that should take place before the
         *        ones for displaying the frame.
         *
         * Virtual.
         */
        virtual void preRender();

        /**
         * @brief Perform subclass-specific rendering operations for displaying the frame.
         *
         * Pure virtual.
         */
        virtual void render(std::weak_ptr<CommandBuffers> cmdBuffer) = 0;

        /**
         * @brief Perform subclass-specific update operations.
         *
         * Pure virtual.
         */
        virtual void update(float deltaSeconds) = 0;

        /**
         * @brief Get the information used to initialize the app.
         *
         * Pure Virtual.
         */
        [[nodiscard]] virtual InitArgs getInitArgs() const = 0;

        /**
         * @brief Rate the suitability of a Vulkan Physical device.
         *
         * This is for choosing the optimal GPU for Vulkan to use.
         * Higher score -> higher priority.
         *
         * Pure Virtual.
         */
        virtual int32_t rateDeviceSuitability(vk::PhysicalDevice device) = 0;

        /**
         * @brief The application state
         */
        AppState state;

    private:
        friend class Renderer;

        InitArgs m_initArgs;
        std::size_t m_currentFrame = 0;

        ///////////////////////////////////////////////////////////////////////
        //                          Private Methods                          //
        ///////////////////////////////////////////////////////////////////////

        /**
         * @brief Run the main application loop
         */
        void mainLoop();

        /**
         * @brief Perform common app initialization.
         */
        void appInitialize();
        /**
         * @brief Perform common app cleanup.
         */
        void appCleanup();

        // ================ SDL and Vulkan ================= //

        void initializeSDL();

        void createVkInstance();

        void createVkSurface();

        std::vector<const char*> getRequiredVkExtensions();
        std::vector<const char*> getRequiredVkDeviceExtensions();
        std::vector<const char*> getRequiredVkLayers();

        bool checkDeviceExtensions(vk::PhysicalDevice device);

        void pickVkPhysicalDevice();
        bool isDeviceSuitable(vk::PhysicalDevice device);

        SwapChainInfo querySwapChainInfo(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseVkSwapFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats);
        VkPresentModeKHR chooseVkPresentMode(const std::vector<VkPresentModeKHR>& supportedModes);
        VkExtent2D chooseVkSwapExtent(const VkSurfaceCapabilitiesKHR& capabilties);
        void createVkSwapChain();
        void cleanupVkSwapChain();
        void recreateVkSwapChain();
        void createVkFramebuffers();
        void createDepthResources();

        void createVkImageViews();

        void createVkDescriptorPool();

        VkDebugUtilsMessengerCreateInfoEXT makeVkDebugMessengerCreateInfo(bool includeVerbose = false);
        void createVkDebugMessenger();

        static VKAPI_ATTR VkBool32 VKAPI_CALL
        debugCallbackVk(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                        void* pUserData);

        QueueFamilyIndices findVkQueueFamilies(VkPhysicalDevice device);

        void createVkLogicalDevice();

        void createVmaAllocator();

        void createVkCommandPools();
        void createVkCommandBuffers(std::size_t imageIndex);

        void createVkSyncObjects();

        void drawFrame();
    };
} // namespace ivulk
