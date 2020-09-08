#include <ivulk/core/app.hpp>
#include <ivulk/core/uniform_buffer.hpp>
#include <ivulk/utils/messages.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <tuple>
using namespace boost::adaptors;

namespace ivulk {
	namespace fs = boost::filesystem;

	const std::map<fs::path, VkShaderStageFlagBits> shaderExtensionToStage = {
		{".vert", VK_SHADER_STAGE_VERTEX_BIT},
		{".frag", VK_SHADER_STAGE_FRAGMENT_BIT},
		{".tesc", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
		{".tese", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
		{".geom", VK_SHADER_STAGE_GEOMETRY_BIT},
		{".comp", VK_SHADER_STAGE_COMPUTE_BIT},
	};

	void App::createVkDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes;

		VkDescriptorPoolSize poolSize {
			.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = static_cast<uint32_t>(state.vk.swapChain.images.size()),
		};
		poolSizes[0]  = poolSize;
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1]  = poolSize;

		VkDescriptorPoolCreateInfo poolInfo {
			.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets       = static_cast<uint32_t>(state.vk.swapChain.images.size()),
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes    = poolSizes.data(),
		};

		if (vkCreateDescriptorPool(state.vk.device, &poolInfo, nullptr, &state.vk.descriptor.pool)
			!= VK_SUCCESS)
			throw std::runtime_error(
				utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan descriptor pool"));
	}

} // namespace ivulk
