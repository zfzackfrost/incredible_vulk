/**
 * @file image.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Image` class.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/vk.hpp>

#include <ivulk/core/vma.hpp>
#include <ivulk/core/vulkan_resource.hpp>

#include <boost/filesystem.hpp>

#include <optional>

namespace ivulk {
    /**
     * @brief Information for initializing an Image resource
     */
    struct ImageInfo final
    {
        /**
         * @brief Settings for loading images from the filesystem.
         */
        struct load
        {
            bool bEnable = false;         ///< Set this to true to load the image from the filesystem
            boost::filesystem::path path; ///< The file path to load the image from
            bool bSrgb
                = true; ///< If true (default), use an sRGB format when loading the image. Otherwise use a normalized format.
            bool bGenMips
                = true; ///< If true (default), generate mipmaps for the loaded image. Otherwise, no mipmaps are generated.
            bool bHDR = false; ///< If true, load the image as an HDR map. Default is false.
        } load;

        VkImageTiling tiling      = VK_IMAGE_TILING_OPTIMAL;    ///< Vulkan image tiling setting
        VkImageUsageFlags usage   = VK_IMAGE_USAGE_SAMPLED_BIT; ///< Vulkan image usage flags
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;  ///< Vulkan image sharing mode
        VmaMemoryUsage memoryMode = E_MemoryMode::GpuOnly;      ///< Memory usage mode
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; ///< Vulkan image format, if not loading from filesystem.
        VkExtent3D extent {};                      ///< The image extent
        VkImageLayout layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; ///< Vulkan image layout
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;                ///< Vulkan image aspect flags
    };

    /**
     * @brief A memory-managed resource for a Vulkan image and image view
     */
    class Image : public VulkanResource<Image, ImageInfo, VkImage, VmaAllocation, VkImageView>
    {
    public:
        /**
         * @brief Get the Vulkan image handle
         */
        VkImage getImage() { return getHandleAt<0>(); }

        /**
         * @brief Get the VMA allocation information.
         */
        VmaAllocation getAllocation() { return getHandleAt<1>(); }

        /**
         * @brief Get the Vulkan image view handle.
         */
        VkImageView getImageView() { return getHandleAt<2>(); }

        /**
         * @brief Get the Vulkan image format.
         */
        VkFormat getFormat() { return m_format; }

        /**
         * @brief Get the Vulkan image extent.
         */
        VkExtent3D getExtent() { return m_extent; }

        void changeLayout(vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage, vk::ImageLayout oldLayout,  vk::ImageLayout newLayout);

    private:
        friend base_t;
        VkFormat m_format;
        VkExtent3D m_extent;
        uint32_t m_mipLevels;

        Image(VkDevice device, VkImage image, VmaAllocation allocation, VkImageView view);

        static Image* createImpl(VkDevice device, ImageInfo createInfo);

        void destroyImpl();

        static uint32_t calcMipLevels(const VkExtent3D extent);
    };
} // namespace ivulk
