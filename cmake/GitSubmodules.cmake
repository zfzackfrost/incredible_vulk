macro(FAIL_GitSubmodules)
    message(
        FATAL_ERROR
            "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again."
    )
endmacro()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${IncredibleVulk_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check git submodules during configuration" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(
                FATAL_ERROR
                    "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
            )
        endif()
    endif()
endif()

if(NOT EXISTS "${IncredibleVulk_SOURCE_DIR}/extern/SDL2/CMakeLists.txt")
    fail_gitsubmodules()
endif()
if(NOT EXISTS "${IncredibleVulk_SOURCE_DIR}/extern/glm/CMakeLists.txt")
    fail_gitsubmodules()
endif()
if(NOT
   EXISTS
   "${IncredibleVulk_SOURCE_DIR}/extern/VulkanMemoryAllocator/src/vk_mem_alloc.h"
)
    fail_gitsubmodules()
endif()
