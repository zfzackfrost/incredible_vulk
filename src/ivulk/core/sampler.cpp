#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/sampler.hpp>

#include <ivulk/core/app.hpp>

namespace ivulk {
    Sampler::Sampler(VkDevice device, VkSampler sampler, VkDescriptorSetLayoutBinding binding)
        : base_t(device, handles_t {sampler, binding})
    { }

    Sampler* Sampler::createImpl(VkDevice device, SamplerInfo info)
    {
        auto state = App::current()->getState().vk;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(state.physicalDevice, &features);

        VkBool32 enableAnisotropy = features.samplerAnisotropy && info.anisotropy.bEnable;
        float maxAnisotropy       = enableAnisotropy ? info.anisotropy.level : 1.0f;

        VkSampler sampler = VK_NULL_HANDLE;
        VkSamplerCreateInfo samplerInfo {
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter               = info.filter.mag,
            .minFilter               = info.filter.min,
            .mipmapMode              = info.mips.mode,
            .addressModeU            = info.addressMode.u,
            .addressModeV            = info.addressMode.v,
            .addressModeW            = info.addressMode.w,
            .mipLodBias              = info.mips.lodBias,
            .anisotropyEnable        = enableAnisotropy,
            .maxAnisotropy           = maxAnisotropy,
            .compareEnable           = info.compare.bEnable,
            .compareOp               = info.compare.compareOp,
            .minLod                  = info.mips.minLod,
            .maxLod                  = info.mips.maxLod,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };
        if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error(
                utils::makeErrorMessage("VK::CREATE", "Failed to create Vulkan sampler."));
        }

        VkDescriptorSetLayoutBinding binding {
            .binding            = info.defaultBinding,
            .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount    = 1,
            .stageFlags         = info.stageFlags,
            .pImmutableSamplers = nullptr,
        };
        return new Sampler(device, sampler, binding);
    }

    void Sampler::destroyImpl() { vkDestroySampler(getDevice(), getSampler(), nullptr); }
} // namespace ivulk
