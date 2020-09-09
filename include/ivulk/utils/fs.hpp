/**
 * @file fs.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Helpers for working with the filesystem.
 */

#pragma once

#include <boost/filesystem.hpp>

#include <optional>

namespace ivulk::utils {

	namespace fs = boost::filesystem;

	std::optional<fs::path> prepareAssetPath(const fs::path& assetPath);
} // namespace ivulk::utils
