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
	template <typename Derived, typename CreateInfo, typename... HandleTypes>
	class VulkanResource
	{
	public:
		using base_t = VulkanResource<Derived, CreateInfo, HandleTypes...>;
		using handles_t = std::tuple<HandleTypes...>;

		VulkanResource(VkDevice device, const std::tuple<HandleTypes...>&& h)
			: m_device(device), handles(h)
		{ }

		virtual ~VulkanResource()
		{
			destroy();
		}

		void destroy()
		{
			static_cast<Derived*>(this)->destroyImpl();
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

		VkDevice getDevice()
		{
			return m_device;
		}
	private:
		VkDevice m_device;
	};
} // namespace ivulk
