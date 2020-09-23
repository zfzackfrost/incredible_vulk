#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/app.hpp>

#include <ivulk/config.hpp>
#include <ivulk/utils/containers.hpp>
#include <ivulk/utils/messages.hpp>
#include <ivulk/utils/table_print.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>

#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>

#include <ivulk/render/renderer.hpp>

namespace ivulk {

    App* s_currentApp = nullptr;

    // NOLINTNEXTLINE
    App::App(int argc, char* argv[])
        : state {}
    {
        state.cmdArgs.reserve(argc);
        for (int i = 0; i < argc; ++i)
            state.cmdArgs.emplace_back(argv[i]);
    }

    void App::run()
    {
        if (current() != nullptr)
            throw std::runtime_error(utils::makeErrorMessage("APP", "Only one app can run at a time!"));

        s_currentApp = this;
        appInitialize();
        mainLoop();
        appCleanup();
        s_currentApp = nullptr;
    }

    App* App::current() { return s_currentApp; }

    AppState App::getState() const { return state; }

    bool App::getPrintDbg() const { return m_initArgs.bDebugPrint; }

    void App::quit() { state.evt.shouldQuit = true; }

    void App::mainLoop()
    {
        using namespace std::chrono;
        auto now           = steady_clock::now();
        auto lastFrameTime = now;

        // Loop until user requests quit...
        while (!state.evt.shouldQuit)
        {
            // Process SDL2 events
            {
                SDL_Event evt;
                while (SDL_PollEvent(&evt) != 0)
                {
                    if (evt.type == SDL_QUIT)
                    {
                        state.evt.shouldQuit = true;
                    }
                    else
                    {
                        EventManager::pushEvent(evt);
                    }
                }
            }

            EventManager::processAllEvents();

            // Render frame
            // drawFrame();
            {
                if (auto r = Renderer::current().lock())
                {
                    r->drawFrame();
                }
            }

            // Update application state
            {
                now                                            = steady_clock::now();
                duration<float, std::ratio<1, 1>> deltaSeconds = (now - lastFrameTime);
                update(deltaSeconds.count());
                lastFrameTime = now;
            }
        }

        vkDeviceWaitIdle(state.vk.device);
        vkQueueWaitIdle(state.vk.queues.present);
        vkQueueWaitIdle(state.vk.queues.graphics);
    }

    ////////////////////////////////////////////////////////////////////////////
    //                             SDL and Vulkan                             //
    ////////////////////////////////////////////////////////////////////////////

    void App::appInitialize()
    {
        // Cache init args
        m_initArgs = getInitArgs();
        state.vk.swapChain.maxFramesInFlight = m_initArgs.vk.maxFramesInFlight;

        // ================== Initialize SDL2 =================== //

        initializeSDL();

        // ================= Initialize Vulkan ================== //

        createVkInstance();
        createVkDebugMessenger();
        createVkSurface();
        pickVkPhysicalDevice();
        createVkLogicalDevice();

        // Create allocator
        createVmaAllocator();

        createVkSwapChain();
        createVkImageViews();
        createVkCommandPools();
        createDepthResources();
        createVkDescriptorPool();

        // Run subclass initialization before creating framebuffers
        initialize();

        createVkFramebuffers();
        createVkSyncObjects();
    }

    void App::appCleanup()
    {

        // Run subclass cleanup
        cleanup(false);

        // =================== Cleanup Vulkan =================== //

        // Destroy sync objects
        for (auto sem : state.vk.sync.imageAvailableSems)
            vkDestroySemaphore(state.vk.device, sem, nullptr);
        for (auto sem : state.vk.sync.renderFinishedSems)
            vkDestroySemaphore(state.vk.device, sem, nullptr);
        for (auto fen : state.vk.sync.inFlightFences)
            vkDestroyFence(state.vk.device, fen, nullptr);

        // Destroy command pools
        vkDestroyCommandPool(state.vk.device, state.vk.cmd.gfxPool, nullptr);

        cleanupVkSwapChain();

        // Destroy VMA allocator
        vmaDestroyAllocator(state.vk.allocator);

        // Destroy logical device
        vkDestroyDevice(state.vk.device, nullptr);

        // Destroy surface
        vkDestroySurfaceKHR(state.vk.instance, state.vk.surface, nullptr);

        // Destroy debug messenger
        utils::ivkDestroyDebugUtilsMessengerEXT(state.vk.instance, state.vk.debugMessenger, nullptr);

        // ... Finally destroy the instance
        vkDestroyInstance(state.vk.instance, nullptr);

        // ==================== Cleanup SDL2 ==================== //

        SDL_DestroyWindow(state.sdl.window);
        SDL_Quit();
    }

    void App::initializeSDL()
    {
        // Throw an exception if init arguments are invalid
        if (m_initArgs.window.width <= 0 || m_initArgs.window.height <= 0)
        {
            throw std::invalid_argument(
                utils::makeErrorMessage("INIT", "Window width and height must be >= 1"));
        }

        // Initialize SDL2 itself
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            throw std::runtime_error(utils::makeErrorMessage("INIT", "Failed to initialize SDL2"));
        }

        // Create SDL2 window for the application
        Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;
        windowFlags |= (m_initArgs.window.bResizable) ? SDL_WINDOW_RESIZABLE : 0u;
        windowFlags |= (m_initArgs.window.bBorderless) ? SDL_WINDOW_BORDERLESS : 0u;
        windowFlags |= (m_initArgs.window.bFullscreen) ? SDL_WINDOW_FULLSCREEN : 0u;

        state.sdl.window = SDL_CreateWindow(m_initArgs.appName.c_str(),
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            m_initArgs.window.width,
                                            m_initArgs.window.height,
                                            windowFlags);

        // Throw exception if window was not created
        // successfully
        if (state.sdl.window == nullptr)
        {
            throw std::runtime_error(utils::makeErrorMessage("INIT", "Failed to create SDL2 window"));
        }

        if (getPrintDbg())
        {
            std::cout << utils::makeSuccessMessage("INIT", "Initialized SDL2") << std::endl;
        }
    }

    void App::createVkInstance()
    {
        auto requiredExtensions = getRequiredVkExtensions();
        auto requiredLayers     = getRequiredVkLayers();

        vk::ApplicationInfo appInfo {};
        appInfo.setPApplicationName(m_initArgs.appName.c_str())
            .setApplicationVersion(m_initArgs.appVersion.toVkVersion())
            .setPEngineName("Incredible Vulk")
            .setEngineVersion(VK_MAKE_VERSION(IVULK_VERSION_MAJOR, IVULK_VERSION_MINOR, IVULK_VERSION_PATCH))
            .setApiVersion(VK_API_VERSION_1_0);

        vk::InstanceCreateInfo createInfo {};
        createInfo.setPApplicationInfo(&appInfo)
            .setEnabledLayerCount(requiredLayers.size())
            .setPpEnabledLayerNames(requiredLayers.data())
            .setEnabledExtensionCount(requiredExtensions.size())
            .setPpEnabledExtensionNames(requiredExtensions.data());

        if (m_initArgs.vk.bEnableValidation)
        {
            auto debugCreateInfo = makeVkDebugMessengerCreateInfo();
            createInfo.setPNext(&debugCreateInfo);
        }

        auto inst_result = vk::createInstance(createInfo);
        if (inst_result.result != vk::Result::eSuccess)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::INIT", "Failed to create the Vulkan instance"));
        }
        else
        {
            state.vk.instance = inst_result.value;
            if (getPrintDbg())
            {
                std::cout << utils::makeSuccessMessage("VK::INIT", " Created the Vulkan instance")
                          << std::endl;
            }
        }

        state.vk.requiredLayers = std::move(requiredLayers);
    }

    void App::pickVkPhysicalDevice()
    {
        auto _devices = state.vk.instance.enumeratePhysicalDevices();
        if (_devices.result != vk::Result::eSuccess || _devices.value.empty())
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::HARDWARE", "Failed to find a Vulkan compatible GPU"));
        }
        auto devices = std::move(_devices.value);

        std::multimap<int32_t, vk::PhysicalDevice> scoredDevices;
        for (auto& d : devices)
        {
            if (isDeviceSuitable(d))
            {
                auto score = rateDeviceSuitability(d);
                scoredDevices.insert(std::make_pair(score, d));
            }
        }
        if (scoredDevices.empty() || scoredDevices.rbegin()->first < 0
            || !scoredDevices.rbegin()->second)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::HARDWARE", "Failed to find a suitable GPU"));
        }
        else
        {
            state.vk.physicalDevice = scoredDevices.rbegin()->second;
            if (getPrintDbg())
            {
                auto deviceProps = state.vk.physicalDevice.getProperties();

                std::string description = "Picked physical device: `";
                description += deviceProps.deviceName.data() + std::string {"`"};

                std::cout << utils::makeInfoMessage("VK::HARDWARE", description) << std::endl;
            }
        }
    }

    bool App::checkDeviceExtensions(vk::PhysicalDevice device)
    {
        auto extensions = device.enumerateDeviceExtensionProperties();
        if (extensions.result != vk::Result::eSuccess)
        {
            throw std::runtime_error(vk::to_string(extensions.result));
        }

        auto deviceExtensions = getRequiredVkDeviceExtensions();

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extProps : extensions.value)
        {
            requiredExtensions.erase(extProps.extensionName.data());
        }

        return requiredExtensions.empty();
    }

    bool App::isDeviceSuitable(vk::PhysicalDevice device)
    {
        QueueFamilyIndices indices = findVkQueueFamilies(device);
        bool extensionsSupported   = checkDeviceExtensions(device);
        bool swapChainOk           = false;
        if (extensionsSupported)
        {
            SwapChainInfo scInfo = querySwapChainInfo(device);
            swapChainOk          = !scInfo.formats.empty() && !scInfo.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainOk;
    }

    std::vector<const char*> App::getRequiredVkDeviceExtensions()
    {
        return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
    }

    void printExtensions(const std::vector<VkExtensionProperties>& extensions)
    {
        utils::Table tbl;
        tbl.push_back({
            "Name",
            "Spec",
        });
        std::transform(extensions.begin(),
                       extensions.end(),
                       std::back_inserter(tbl),
                       [](VkExtensionProperties extProps) -> utils::TableRow {
                           auto ver = utils::VersionData::fromVkVersion(extProps.specVersion);
                           return {
                               std::string(extProps.extensionName),
                               ver.toString(),
                           };
                       });
        tbl.enableHeadingRow().leadingSpaces(3);
        std::cout << "~~~ Supported Extensions: " << std::endl;
        std::cout << tbl << std::endl << std::endl;
    }

    std::vector<const char*> App::getRequiredVkExtensions()
    {
        uint32_t sdlExtCount = 0;
        if (SDL_Vulkan_GetInstanceExtensions(state.sdl.window, &sdlExtCount, nullptr) == SDL_FALSE)
        {
            throw std::runtime_error(utils::makeErrorMessage(
                "VK::EXT", "Failed to get the count of required vulkan extensions from SDL2"));
        }
        std::vector<const char*> requiredExtensions(sdlExtCount);
        if (SDL_Vulkan_GetInstanceExtensions(state.sdl.window, &sdlExtCount, requiredExtensions.data())
            == SDL_FALSE)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::EXT", "Failed to get required vulkan extensions from SDL2"));
        }

        // Get all supported extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        // Debug print supported extensions
        if (getPrintDbg())
        {
            printExtensions(extensions);
        }

        // Additional extenstions, based on init args
        if (m_initArgs.vk.bEnableValidation)
        {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return requiredExtensions;
    }

    void printLayers(const std::vector<VkLayerProperties>& layers)
    {
        utils::Table tbl;
        tbl.push_back({
            "Name",
            "Spec",
            "Impl",
            "Description",
        });
        std::transform(layers.begin(),
                       layers.end(),
                       std::back_inserter(tbl),
                       [](VkLayerProperties lyrProps) -> utils::TableRow {
                           auto verSpec = utils::VersionData::fromVkVersion(lyrProps.specVersion);
                           auto verImpl = utils::VersionData::fromVkVersion(lyrProps.implementationVersion);
                           return {std::string(lyrProps.layerName),
                                   verSpec.toString(),
                                   verImpl.toString(),
                                   std::string(lyrProps.description)};
                       });
        tbl.enableHeadingRow().leadingSpaces(3);
        std::cout << "~~~ Supported Layers: " << std::endl;
        std::cout << tbl << std::endl << std::endl;
    }

    std::vector<const char*> App::getRequiredVkLayers()
    {
        std::vector<const char*> requiredLayers;
        if (m_initArgs.vk.bEnableValidation)
        {
            requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
        }

        // Get all supported layers
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

        // Debug print layers
        if (getPrintDbg())
        {
            printLayers(layers);
        }

        // Make sure all required layers are supported
        {
            for (auto& layerName : requiredLayers)
            {
                auto found = std::find_if(layers.begin(),
                                          layers.end(),
                                          [layerName](const VkLayerProperties& lyrProps) {
                                              return std::strcmp(layerName, lyrProps.layerName) == 0;
                                          })
                             != layers.end();
                if (!found)
                {
                    std::string description = "Required Vulkan layer is not supported: `";
                    description += layerName + std::string {"`"};
                    throw std::runtime_error(utils::makeErrorMessage("VK::LAYER", description));
                }
            }
        }

        return requiredLayers;
    }

    VkDebugUtilsMessengerCreateInfoEXT App::makeVkDebugMessengerCreateInfo(bool includeVerbose)
    {
        return {
            .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = (includeVerbose ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : 0u)
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                           | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallbackVk,
            .pUserData       = nullptr,
        };
    }

    void App::createVkDebugMessenger()
    {
        if (!m_initArgs.vk.bEnableValidation)
            return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo = makeVkDebugMessengerCreateInfo(false);

        if (utils::ivkCreateDebugUtilsMessengerEXT(
                state.vk.instance, &createInfo, nullptr, &state.vk.debugMessenger)
            != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create debug messenger"));
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL
    App::debugCallbackVk(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                         void* pUserData)
    {
        std::cout << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    QueueFamilyIndices App::findVkQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, families.data());

        uint32_t idx = 0;
        for (const auto& qf : families)
        {
            // Check for graphics support
            if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics = idx;
            }

            // Check for present support
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, state.vk.surface, &presentSupport);
            if (presentSupport)
            {
                indices.present = idx;
            }

            // Early exit
            if (indices.isComplete())
            {
                break;
            }
            idx++;
        }

        return indices;
    }

    void App::createVkLogicalDevice()
    {
        float queuePriority        = 1.0f;
        QueueFamilyIndices indices = findVkQueueFamilies(state.vk.physicalDevice);
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

        // Collect queue create infos
        {
            vk::DeviceQueueCreateInfo createInfoBase;
            createInfoBase.setQueueCount(1).setPQueuePriorities(&queuePriority);
            auto uniqueQueueFamilies = indices.getUniqueFamilies();
            for (const auto& i : uniqueQueueFamilies)
            {
                auto createInfo = createInfoBase;
                createInfo.setQueueFamilyIndex(i);
                queueCreateInfos.push_back(createInfo);
            }
        }

        auto deviceExtensions = getRequiredVkDeviceExtensions();

        // ============= Collect features ============== //

        vk::PhysicalDeviceFeatures supportedFeatures = state.vk.physicalDevice.getFeatures();

        vk::PhysicalDeviceFeatures deviceFeatures {};
        deviceFeatures.setSamplerAnisotropy(supportedFeatures.samplerAnisotropy);

        // =========== Create logical device =========== //

        vk::DeviceCreateInfo createInfo {};
        createInfo.setQueueCreateInfoCount(queueCreateInfos.size())
            .setPQueueCreateInfos(queueCreateInfos.data())
            .setEnabledExtensionCount(deviceExtensions.size())
            .setPpEnabledExtensionNames(deviceExtensions.data())
            .setPEnabledFeatures(&deviceFeatures);
        if (m_initArgs.vk.bEnableValidation)
        {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(state.vk.requiredLayers.size());
            createInfo.ppEnabledLayerNames = state.vk.requiredLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        auto r = state.vk.physicalDevice.createDevice(createInfo);
        if (r.result != vk::Result::eSuccess)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create logical device"));
        }
        else
        {
            state.vk.device = r.value;
            if (getPrintDbg())
            {
                std::cout << utils::makeSuccessMessage("VK::CREATE", "Created the Vulkan logical device")
                          << std::endl;
            }
        }

        // Get queue handles
        state.vk.queues.graphics = state.vk.device.getQueue(indices.graphics.value(), 0);
        state.vk.queues.present  = state.vk.device.getQueue(indices.present.value(), 0);
    }

    void App::createVmaAllocator()
    {
        VmaAllocatorCreateInfo createInfo {.physicalDevice = state.vk.physicalDevice,
                                           .device         = state.vk.device,
                                           .instance       = state.vk.instance};
        if (vmaCreateAllocator(&createInfo, &state.vk.allocator) != VK_SUCCESS)
            throw std::runtime_error(
                utils::makeErrorMessage("VK::MEM", "Failed to create VMA allocator for Vulkan"));
    }

} // namespace ivulk
