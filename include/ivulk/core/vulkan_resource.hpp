/**
 * @file vulkan_resource.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `VulkanResource` class.
 */

#pragma once

#include <ivulk/config.hpp>

#include <tuple>
#include <ivulk/vk.hpp>

#include <memory>

namespace ivulk {
    /**
     * @brief Empty information for initializing a resource.
     */
    struct NullResourceInfo final
    { };

    /**
     * @brief Base class template for memory-managed wrapper classes for Vulkan resources
     *
     * @tparam Derived A derived type (for CRTP)
     * @tparam CreateInfo The structure used to provide initialization 
     *                    information for the derived type
     * @tparam HandleTypes Parameter pack where each type corresponds to 
     *                     some arbitrary data stored in a tuple in the 
     *                     resource. The data is primarily Vulkan handles,
     *                     but any type is allowed.
     */
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

        /**
         * @brief Constructor
         *
         * @param device The Vulkan device associated with this resource
         * @param h The tuple containing the handle and/or other data for this resource
         */
        VulkanResource(VkDevice device, const std::tuple<HandleTypes...>&& h)
            : m_device(device)
            , handles(h)
        { }

        /**
         * @brief Virtual destructor
         *
         * Calls `destroy()`
         */
        virtual ~VulkanResource() { destroy(); }


        /**
         * @brief Destroys the resource.
         *
         *
         * If `destroy()` has already been called, this has no effect.
         *
         * @note
         * Calls a non-static member function `Derived::destroyImpl()` on
         * `this`. 
         */
        void destroy()
        {
            if (m_destroyed)
                return;
            static_cast<Derived*>(this)->destroyImpl();
            m_destroyed = true;
        }

        /**
         * @brief Gets a value from the handle/data tuple by index
         *
         * @tparam I The index of the handle/data to get
         */
        template <std::size_t I>
        auto getHandleAt()
        {
            return std::get<I>(handles);
        }

        /**
         * @brief Create a new instance from handles
         *
         * @param device The device to associate with the new resource
         * @param args The values to fill the handles/data tuple with
         */
        static std::shared_ptr<Derived> fromHandles(VkDevice device, HandleTypes... args)
        {
            return std::shared_ptr<Derived>(new Derived(device, args...));
        }

        /**
         * @brief Create a new instance using initialization information
         *
         * @param device The device to associate with the new resource
         * @param createInfo The initialization info to create a new instance with
         */
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
