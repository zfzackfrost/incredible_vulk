/**
 * @file app.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `App` class and related.
 */

#pragma once

#include <ivulk/core/app_state.hpp>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>
#include <ivulk/core/swap_chain.hpp>
#include <ivulk/utils/version_data.hpp>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace ivulk {

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
		App(int argc, char* argv[]);

		/**
		 * @brief Virtual destructor
		 */
		virtual ~App();

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
		AppState getState() const;

	protected:
		/**
		 * @brief Information used when initializing the app
		 */
		struct InitArgs
		{
			std::string appName = "";
			utils::VersionData appVersion = {0, 1, 0};
			bool bDebugPrint = false;
			struct
			{
				int width = -1;
				int height = -1;
				bool bResizable = false;
				bool bBorderless = false;
				bool bFullscreen = false;
			} window;
			struct
			{
				bool bEnableValidation = false;
				std::size_t maxFramesInFlight = 2;
			} vk;
		};

		/**
		 * @brief Perform subclass-specific initialization.
		 *
		 * Pure virtual.
		 */
		virtual void initialize() = 0;

		/**
		 * @brief Perform subclass-specific cleanup.
		 *
		 * Pure virtual.
		 */
		virtual void cleanup() = 0;

		/**
		 * @brief Perform subclass-specific rendering operations.
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
		 * @brief Get the information used to intialize the app.
		 *
		 * Pure Virtual.
		 */
		virtual InitArgs getInitArgs() const = 0;

		/**
		 * @brief Rate the suitability of a Vulkan Physical device.
		 *
		 * This is for choosing the optimal GPU for Vulkan to use.
		 * Higher score -> higher priority.
		 *
		 * Pure Virtual.
		 */
		virtual int32_t rateDeviceSuitability(VkPhysicalDevice device) = 0;

		/**
		 * @brief The application state
		 */
		AppState state;

		/**
		 * @brief Helper method to check if debug printing was enabled in the init args.
		 */
		bool getPrintDbg() const;

		/**
		 * @brief Get the path to the assets directory
		 *
		 * Pure Virtual
		 */
		virtual std::filesystem::path getAssetsDir() = 0;

		/**
		 * @brief Create a graphics pipeline using the shaders at the specified asset paths.
		 */
		std::shared_ptr<GraphicsPipeline>
		createVkGraphicsPipeline(const std::vector<std::filesystem::path>& shaderPaths,
								 const VkVertexInputBindingDescription bindingDescr,
								 const std::vector<VkVertexInputAttributeDescription>& attribDescrs);

	private:
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

		bool checkDeviceExtensions(VkPhysicalDevice device);

		void pickVkPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);

		SwapChainInfo querySwapChainInfo(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseVkSwapFormat(const std::vector<VkSurfaceFormatKHR>& supportedFormats);
		VkPresentModeKHR chooseVkPresentMode(const std::vector<VkPresentModeKHR>& supportedModes);
		VkExtent2D chooseVkSwapExtent(const VkSurfaceCapabilitiesKHR& capabilties);
		void createVkSwapChain();
		void createVkFramebuffers();

		void createVkImageViews();

		VkShaderModule createShaderModule(const std::vector<char>& shaderCode,
										  const std::filesystem::path& assetPath);

		VkDebugUtilsMessengerCreateInfoEXT makeVkDebugMessengerCreateInfo(bool includeVerbose = false);
		void createVkDebugMessenger();

		static VKAPI_ATTR VkBool32 VKAPI_CALL
		debugCallbackVk(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
						VkDebugUtilsMessageTypeFlagsEXT messageType,
						const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		QueueFamilyIndices findVkQueueFamilies(VkPhysicalDevice device);

		void createVkLogicalDevice();

		void createVkCommandPools();
		void createVkCommandBuffers(std::size_t imageIndex);

		void createVkSyncObjects();

		void drawFrame();
	};
} // namespace ivulk
