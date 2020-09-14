#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/utils/format.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/utils/messages.hpp>

#include <stdexcept>

namespace ivulk::utils {
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features)
    {
        auto device = App::current()->getState().vk.physicalDevice;

        for (const auto& format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(device, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL
                     && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }
        throw std::runtime_error(utils::makeErrorMessage("VK::FIND", "Failed to find supported VkFormat"));
    }

    VkFormat findDepthFormat()
    {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
    bool hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }
} // namespace ivulk::utils
