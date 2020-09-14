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
        std::vector<VkImageView> attachments;
        attachments.reserve(info.attachments.size());
        std::transform(info.attachments.begin(),
                       info.attachments.end(),
                       std::back_inserter(attachments),
                       [](info_att_t att) -> VkImageView {
                           if (att.index() == 0)
                           {
                               if (auto a = std::get<0>(att).lock())
                                   return a->getImageView();
                           }
                           if (att.index() == 1)
                           {
                               if (auto a = std::get<1>(att))
                                   return a->getImageView();
                           }
                           if (att.index() == 2)
                           {
                               if (auto a = std::get<2>(att))
                                   return a;
                           }
                           return VK_NULL_HANDLE;
                       });

        VkRenderPass renderPass = VK_NULL_HANDLE;
        if (info.renderContext.index() == 0)
        {
            if (auto a = std::get<0>(info.renderContext).lock())
                renderPass = a->getRenderPass();
        }
        else if (info.renderContext.index() == 1)
        {
            if (auto a = std::get<1>(info.renderContext))
                renderPass = a->getRenderPass();
        }
        else if (info.renderContext.index() == 2)
        {
            if (auto a = std::get<2>(info.renderContext))
                renderPass = a;
        }

        VkFramebufferCreateInfo createInfo {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments    = attachments.data(),
            .width           = info.width,
            .height          = info.height,
            .layers          = info.layers,
        };

        VkFramebuffer fbuf;
        if (vkCreateFramebuffer(device, &createInfo, nullptr, &fbuf) != VK_SUCCESS)
        {
            throw std::runtime_error(utils::makeErrorMessage("VK::CREATE", "Failed to create framebuffer"));
        }

        return new Framebuffer(device, fbuf);
    }

    void Framebuffer::destroyImpl() { vkDestroyFramebuffer(getDevice(), getFramebuffer(), nullptr); }
} // namespace ivulk
