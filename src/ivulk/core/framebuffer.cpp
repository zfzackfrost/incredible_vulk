#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/core/framebuffer.hpp>
#include <ivulk/utils/messages.hpp>

#include <algorithm>
#include <stdexcept>

namespace ivulk {
    Framebuffer::Framebuffer(VkDevice device, VkFramebuffer fbuf)
        : base_t(device, handles_t {fbuf})
    { }

    Framebuffer* Framebuffer::createImpl(const VkDevice device, const FramebufferInfo info)
    {
        using info_att_t = decltype(info.attachments.at(0));
        std::vector<vk::ImageView> attachments;
        attachments.reserve(info.attachments.size());
        std::transform(info.attachments.begin(),
                       info.attachments.end(),
                       std::back_inserter(attachments),
                       [](info_att_t att) -> VkImageView {
                           if (att.index() == 0)
                           {
                               if (auto a = std::get<0>(att).lock())
                                   return vk::ImageView(a->getImageView());
                           }
                           else if (att.index() == 1)
                           {
                               if (auto a = std::get<1>(att))
                                   return vk::ImageView(a->getImageView());
                           }
                           else if (att.index() == 2)
                           {
                               if (auto a = std::get<2>(att))
                                   return a;
                           }
                           return vk::ImageView(nullptr);
                       });

        vk::RenderPass renderPass {nullptr};
        if (info.renderContext.index() == 0)
        {
            if (auto a = std::get<0>(info.renderContext).lock())
                renderPass = vk::RenderPass(a->getRenderPass());
        }
        else if (info.renderContext.index() == 1)
        {
            if (auto a = std::get<1>(info.renderContext))
                renderPass = vk::RenderPass(a->getRenderPass());
        }
        else if (info.renderContext.index() == 2)
        {
            if (auto a = std::get<2>(info.renderContext))
                renderPass = a;
        }

        vk::FramebufferCreateInfo createInfo {};
        createInfo.setAttachmentCount(attachments.size());
        createInfo.setPAttachments(attachments.data());
        createInfo.setRenderPass(renderPass);
        createInfo.setWidth(info.width);
        createInfo.setHeight(info.height);
        createInfo.setLayers(info.layers);

        VkFramebufferCreateInfo ci = createInfo;
        VkFramebuffer fbuf;
        if (vkCreateFramebuffer(device, &ci, nullptr, &fbuf) != VK_SUCCESS)
        {
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", "Failed to create framebuffer"));
        }

        return new Framebuffer(device, fbuf);
    }

    void Framebuffer::destroyImpl() { vkDestroyFramebuffer(getDevice(), getFramebuffer(), nullptr); }
} // namespace ivulk
