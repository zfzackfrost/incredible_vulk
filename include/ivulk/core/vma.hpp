/**
 * @file vma.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Header file to include `VulkanMemoryAllocator` library.
 */

#pragma once

#include <vk_mem_alloc.h>

namespace ivulk {
	namespace E_MemoryMode {
			constexpr VmaMemoryUsage Unknown = VMA_MEMORY_USAGE_UNKNOWN;
			constexpr VmaMemoryUsage GpuOnly = VMA_MEMORY_USAGE_GPU_ONLY;
			constexpr VmaMemoryUsage CpuOnly = VMA_MEMORY_USAGE_CPU_ONLY;
			constexpr VmaMemoryUsage CpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU;
			constexpr VmaMemoryUsage GpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU;
			constexpr VmaMemoryUsage CpuCopy = VMA_MEMORY_USAGE_CPU_COPY;
			constexpr VmaMemoryUsage GpuLazy = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
	} // namespace E_BufferMemoryMode
} // namespace ivulk
