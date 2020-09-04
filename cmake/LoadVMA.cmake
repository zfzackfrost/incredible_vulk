######################################################################
#                   Vulkan Memory Allocator Target                   #
######################################################################

add_library(Vma INTERFACE IMPORTED)

######################################################################
#                          Configure Target                          #
######################################################################

set(IVULK_VMA_ROOT_DIR "${IVULK_EXTERN_DIR}/VulkanMemoryAllocator")
set(IVULK_VMA_INCLUDE_DIR "${IVULK_VMA_ROOT_DIR}/src")

target_include_directories(Vma INTERFACE "${IVULK_VMA_INCLUDE_DIR}")
