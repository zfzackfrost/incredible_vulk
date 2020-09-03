/**
 * @file vertex.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Macros for defining vertex structures.
 */

#pragma once

#include <vulkan/vulkan.h>

#include <boost/preprocessor.hpp>

#include <array>

#include <glm/glm.hpp>

#define _IVULK_VERTEX_ATTRIB_EACH(r, data, elem) BOOST_PP_TUPLE_ELEM(0, elem) BOOST_PP_TUPLE_ELEM(1, elem);

#define _IVULK_VERTEX_ATTRIB_DESCR_EACH(r, data, i, elem)                                                    \
	descrs[i].binding = binding;                                                                             \
	descrs[i].format = ::ivulk::VertexAttribTypeFormat<BOOST_PP_TUPLE_ELEM(0, elem)>::value;                 \
	descrs[i].location = BOOST_PP_TUPLE_ELEM(2, elem);                                                       \
	descrs[i].offset = offsetof(data, BOOST_PP_TUPLE_ELEM(1, elem));

#define IVULK_VERTEX_STRUCT(structName, attribs)                                                             \
	struct structName : public ::ivulk::VertexBase<structName>                                               \
	{                                                                                                        \
		static constexpr std::size_t N_ATTRIBS = BOOST_PP_SEQ_SIZE(attribs);                                 \
		BOOST_PP_SEQ_FOR_EACH(_IVULK_VERTEX_ATTRIB_EACH, structName, attribs)                                \
                                                                                                             \
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding = 0) \
		{                                                                                                    \
			std::vector<VkVertexInputAttributeDescription> descrs(N_ATTRIBS);                                \
			BOOST_PP_SEQ_FOR_EACH_I(_IVULK_VERTEX_ATTRIB_DESCR_EACH, structName, attribs)                    \
			return descrs;                                                                                   \
		}                                                                                                    \
	}

#define IVULK_VERTEX_ATTRIB_TYPE_FORMAT(attribType, fmt)                                                     \
	template <>                                                                                              \
	struct VertexAttribTypeFormat<attribType>                                                                \
	{                                                                                                        \
		static constexpr VkFormat value = fmt;                                                               \
	}

namespace ivulk {
	template <typename Derived>
	struct VertexBase
	{
		static VkVertexInputBindingDescription
		getBindingDescription(const uint32_t binding = 0u,
							  const VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX)
		{
			VkVertexInputBindingDescription bindingDescr {
				.binding = binding,
				.stride = sizeof(Derived),
				.inputRate = inputRate,
			};
			return bindingDescr;
		}
	};

	template <typename T>
	struct VertexAttribTypeFormat
	{ };

	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(float, VK_FORMAT_R32_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(double, VK_FORMAT_R64_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(int, VK_FORMAT_R32_SINT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(uint32_t, VK_FORMAT_R32_UINT);

	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::f32vec2, VK_FORMAT_R32G32_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::f64vec2, VK_FORMAT_R64G64_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::i32vec2, VK_FORMAT_R32G32_SINT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::u32vec2, VK_FORMAT_R32G32_UINT);

	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::f32vec3, VK_FORMAT_R32G32B32_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::f64vec3, VK_FORMAT_R64G64B64_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::i32vec3, VK_FORMAT_R32G32B32_SINT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::u32vec3, VK_FORMAT_R32G32B32_UINT);

	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::f32vec4, VK_FORMAT_R32G32B32A32_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::f64vec4, VK_FORMAT_R64G64B64A64_SFLOAT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::i32vec4, VK_FORMAT_R32G32B32A32_SINT);
	IVULK_VERTEX_ATTRIB_TYPE_FORMAT(glm::u32vec4, VK_FORMAT_R32G32B32A32_UINT);

} // namespace ivulk
