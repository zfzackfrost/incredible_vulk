/**
 * @file uniform_buffer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `UniformBufferObject` class.
 */

#pragma once

#include <ivulk/core/buffer.hpp>
#include <ivulk/core/shader_stage.hpp>

#include <iomanip>
#include <typeindex>

namespace ivulk {
    struct UniformBufferObjectInfo final
    {
        VkDeviceSize size             = 0u;
        VmaMemoryUsage memoryMode     = E_MemoryMode::CpuToGpu;
        VkShaderStageFlags stageFlags = E_ShaderStage::AllGraphics;
        uint32_t defaultBinding       = 0u;
    };

    class UniformBufferObject : public VulkanResource<UniformBufferObject,
                                                      UniformBufferObjectInfo,
                                                      Buffer::Ptr,
                                                      VkDeviceSize,
                                                      VkDescriptorSetLayoutBinding>
    {
    public:
        VkBuffer getBuffer() { return getHandleAt<0>()->getBuffer(); }
        VkDeviceSize getSize() { return getHandleAt<1>(); }
        VkDescriptorSetLayoutBinding getDescriptorSetLayoutBinding(uint32_t bindingIndex)
        {
            auto descrBinding    = getHandleAt<2>();
            descrBinding.binding = bindingIndex;
            return descrBinding;
        }

        template <typename BufferData>
        void setUniforms(const BufferData bufData)
        {
            constexpr VkDeviceSize SZ = sizeof(BufferData);
            if (SZ != getSize())
            {
                throw std::runtime_error(utils::makeErrorMessage(
                    "VK::UNIFORM",
                    "Supplied buffer data does not match the size of the `UniformBufferObject`"));
            }
            if (auto buf = getHandleAt<0>())
            {
                BufferData data[] = {bufData};
                buf->fillBuffer(data, SZ);
            }
        }

    private:
        friend base_t;

        UniformBufferObject(VkDevice device,
                            Buffer::Ptr buffer,
                            VkDeviceSize sz,
                            VkDescriptorSetLayoutBinding descrBinding)
            : base_t(device, handles_t {buffer, sz, descrBinding})
        { }

        static UniformBufferObject* createImpl(VkDevice device, UniformBufferObjectInfo createInfo)
        {
            auto buffer = Buffer::create(device,
                                         {
                                             .size       = createInfo.size,
                                             .usage      = E_BufferUsage::Uniform,
                                             .memoryMode = createInfo.memoryMode,
                                         });
            VkDescriptorSetLayoutBinding descrBinding {
                .binding         = createInfo.defaultBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = createInfo.stageFlags,
            };
            return new UniformBufferObject(device, buffer, createInfo.size, descrBinding);
        }

        void destroyImpl()
        {
            if (auto b = getHandleAt<0>())
                b->destroy();
        }
    };

    struct PipelineUniformBufferBinding final
    {
        UniformBufferObject::Ref ubo;
        uint32_t binding;

        VkDescriptorSetLayoutBinding getDescriptorSetLayoutBinding()
        {
            VkDescriptorSetLayoutBinding result;
            if (auto r = ubo.lock())
            {
                result = r->getDescriptorSetLayoutBinding(binding);
            }
            return result;
        }

        VkBuffer getBuffer() const
        {
            if (auto r = ubo.lock())
            {
                return r->getBuffer();
            }
            return VK_NULL_HANDLE;
        }
        VkDeviceSize getSize() const
        {
            if (auto r = ubo.lock())
            {
                return r->getSize();
            }
            return 0u;
        }
    };
} // namespace ivulk
