/**
 * @file messages.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Utility functions for messages and logging.
 */

#pragma once

#include <ivulk/config.hpp>

#include <iostream>
#include <string>
#include <ivulk/vk.hpp>

namespace ivulk::utils {

	std::string makeMessageBase(const char* tag, char marker, std::string type, std::string description);

	std::string makeErrorMessage(std::string type, std::string description);
	std::string makeWarningMessage(std::string type, std::string description);
	std::string makeInfoMessage(std::string type, std::string description);
	std::string makeSuccessMessage(std::string type, std::string description);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
														uint64_t srcObject, size_t location, int32_t iMsgCode,
														const char* pLayerPrefix, const char* pMsg,
														void* pUserData)
	{
		std::string layerPrefix {pLayerPrefix};
		std::string msgCode = std::to_string(iMsgCode);
		std::string type = layerPrefix + "-" + msgCode;

		if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			std::cout << makeErrorMessage(type, pMsg);
		else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			std::cout << makeWarningMessage(type, pMsg);
		else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			std::cout << makeInfoMessage(type, pMsg);
		else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			std::cout << makeMessageBase("DEBUG", '~', type, pMsg);
		else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			std::cout << makeMessageBase("PERFORM", '~', type, pMsg);
		else
			std::cout << makeMessageBase("UNKNOWN", '?', type, pMsg);

		std::cout << std::endl;
		/*
		 * False indicates that layer should not bail-out of an
		 * API call that had validation failures. This may mean that the
		 * app dies inside the driver due to invalid parameter(s).
		 * That's what would happen without validation layers, so we'll
		 * keep that behavior here.
		 */
		return false;
	}

	inline VkResult ivkCreateDebugUtilsMessengerEXT(VkInstance instance,
											 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
											 const VkAllocationCallbacks* pAllocator,
											 VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	inline void ivkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
									   const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
} // namespace ivulk::utils
