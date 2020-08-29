cmake_minimum_required(VERSION 3.15)
project(IncredibleVulk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(GitSubmodules)

######################################################################
#                        Build Configuration                         #
######################################################################

option(IVULK_BUILD_EXAMPLES "Build Incredible Vulk example applications." ON)

######################################################################
#                            Dependencies                            #
######################################################################

include(LoadVulkanSDK)

# =================== SDL2 ==================== #

set(SDL_STATIC_PIC ON CACHE BOOL "")
add_subdirectory(extern/SDL2)

######################################################################
#                              Targets                               #
######################################################################

# ================ Main Target ================ #

add_subdirectory(include)
add_subdirectory(src)

# ================= Examples ================== #

if(IVULK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
