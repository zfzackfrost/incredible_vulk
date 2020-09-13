/**
 * @file vulkan_resource.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `VulkanResource` class.
 */

#pragma once

#include <tuple>
#include <vulkan/vulkan.h>

#include <memory>

namespace ivulk {
    struct NullResourceInfo final
    { };

    template <typename Derived, typename CreateInfo, typename... HandleTypes>
    class VulkanResource
    {
    public:
        /**
		 * @brief Shared pointer to the resource
		 */
        using Ptr = std::shared_ptr<Derived>;

        /**
		 * @brief Weak reference to the resource
		 */
        using Ref = std::weak_ptr<Derived>;

        using base_t    = VulkanResource<Derived, CreateInfo, HandleTypes...>;
        using handles_t = std::tuple<HandleTypes...>;

        VulkanResource(VkDevice device, const std::tuple<HandleTypes...>&& h)
            : m_device(device)
            , handles(h)
        { }

        virtual ~VulkanResource() { destroy(); }

        void destroy()
        {
            if (m_destroyed)
                return;
            static_cast<Derived*>(this)->destroyImpl();
            m_destroyed = true;
        }

        template <std::size_t I>
        auto getHandleAt()
        {
            return std::get<I>(handles);
        }

        static std::shared_ptr<Derived> fromHandles(VkDevice device, HandleTypes... args)
        {
            return std::shared_ptr<Derived>(new Derived(device, args...));
        }

        static std::shared_ptr<Derived> create(VkDevice device, const CreateInfo& createInfo)
        {
            return std::shared_ptr<Derived>(Derived::createImpl(device, createInfo));
        }

    protected:
        handles_t handles;

        VkDevice getDevice() { return m_device; }
        void setDestroyed(bool bDestroyed) { m_destroyed = bDestroyed; }

    private:
        VkDevice m_device;
        bool m_destroyed = false;
    };
} // namespace ivulk
