######################################################################
#                        Find Vulkan Package                         #
######################################################################

find_package(Vulkan REQUIRED)

######################################################################
#                    Find Shader Compiler Package                    #
######################################################################

get_filename_component(TMP ${Vulkan_INCLUDE_DIR} DIRECTORY)
set(Vulkan_ROOT_DIR "${TMP}" CACHE PATH "Vulkan SDK root directory.")

find_program(GLSLC_EXECUTABLE
    NAMES glslc
    PATHS ${Vulkan_ROOT_DIR}
    PATH_SUFFIXES bin Bin Bin32
    REQUIRED
    )

message(STATUS "Found glslc: '${GLSLC_EXECUTABLE}'")

