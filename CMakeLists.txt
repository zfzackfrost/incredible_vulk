cmake_minimum_required(VERSION 3.15)
project(IncredibleVulk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# ================ Directories ================= #

set(IVULK_EXTERN_DIR "${PROJECT_SOURCE_DIR}/extern")

# =========== Update Git Submodules ============ #

include(GitSubmodules)

set(BUILD_SHARED_LIBS ON CACHE BOOL "")
######################################################################
#                        Build Configuration                         #
######################################################################

option(IVULK_DOWNLOAD_MISSING "Download missing dependencies. Otherwise build will fail with missing dependencies." ON)
option(IVULK_BUILD_EXAMPLES "Build Incredible Vulk example applications." ON)

######################################################################
#                            Dependencies                            #
######################################################################

include(LoadBoost)
include(LoadGLM)
include(LoadVulkanSDK)
include(LoadVMA)
include(LoadSTB)
include(LoadAssimp)

# CMake utilities
include(ProcessContent)

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
