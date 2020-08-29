/**
 * @file version_data.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `VersionData` structure.
 */

#pragma once

#include <cstdint>
#include <string>

namespace ivulk::utils {
	/**
	 * @brief Structure for holding a Vulkan-compatible version
	 */
	struct VersionData
	{
		uint32_t major; ///< Major version number
		uint32_t minor; ///< Minor version number
		uint32_t patch; ///< Patch version number

		/**
		 * @brief Convert to a Vulkan-compatible version number
		 */
		uint32_t toVkVersion() const
		{
			return (((major) << 22) | ((minor) << 12) | (patch));
		}

		std::string toString() const
		{
			auto mj = std::to_string(major);
			auto mn = std::to_string(minor);
			auto pt = std::to_string(patch);
			return mj + "." + mn + "." + pt;
		}

		/**
		 * @brief Create from Vulkan-compatible version number
		 */
		static VersionData fromVkVersion(uint32_t version)
		{
			return {
				.major = version >> 22,
				.minor = (version >> 12) & 0x3ff,
				.patch = version & 0xfff
			};
		}
	};
} // namespace ivulk::utils
