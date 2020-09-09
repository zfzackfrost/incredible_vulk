#include <ivulk/utils/fs.hpp>

#include <ivulk/core/app.hpp>

namespace ivulk::utils {
	std::optional<fs::path> prepareAssetPath(const fs::path& assetPath)
	{
		auto assetsDir = App::current()->getAssetsDir();
		auto modelPath = assetPath;
		if (modelPath.is_relative())
			modelPath = assetsDir / modelPath;
		if (!fs::exists(modelPath) || fs::is_directory(modelPath))
		{
			return {};
		}
		return modelPath;
	}
} // namespace ivulk::utils
