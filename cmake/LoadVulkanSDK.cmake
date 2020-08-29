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

######################################################################
#                      Compile Shaders Command                       #
######################################################################

set(SHADER_SOURCE_EXTENSIONS "")
list(APPEND SHADER_SOURCE_EXTENSIONS ".frag")
list(APPEND SHADER_SOURCE_EXTENSIONS ".vert")
list(APPEND SHADER_SOURCE_EXTENSIONS ".tesc")
list(APPEND SHADER_SOURCE_EXTENSIONS ".tese")
list(APPEND SHADER_SOURCE_EXTENSIONS ".geom")
list(APPEND SHADER_SOURCE_EXTENSIONS ".comp")
list(APPEND SHADER_SOURCE_EXTENSIONS ".hlsl")


macro(ProcessShaderResource)
    set(TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets/${OUT})
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/assets/${OUT}/${FNAME}.spv")
    get_filename_component(OUT_DIR ${OUT} DIRECTORY)
    add_custom_command(
        COMMENT "Compiling shader file: '${FNAME}'"
        OUTPUT ${OUT}
        DEPENDS ${CUR_RESOURCE}
        COMMAND mkdir -p ${OUT_DIR} && ${GLSLC_EXECUTABLE} "${CUR_RESOURCE}" -o "${OUT}"
    )

    list(APPEND TMP_RESOURCE_LIST ${CUR_RESOURCE} ${OUT})
endmacro()

# Arguments:
#   TargetName: The name of the custom target to create for processing content
#   ContentDir[Optional]: If specified, overrides the content directory.
function(ConfigureContent TargetName)
    if (ARGC GREATER 1)
        set(TMP_CONTENT_DIR ${ARGV1})
    else()
        set(TMP_CONTENT_DIR ${PROJECT_SOURCE_DIR}/assets)
    endif()

    file(GLOB_RECURSE PROJ_CONTENT "${TMP_CONTENT_DIR}/*.*")
    set(TMP_RESOURCE_LIST "")

    foreach (CUR_RESOURCE ${PROJ_CONTENT})
        get_filename_component(FNAME ${CUR_RESOURCE} NAME)
        get_filename_component(DIR ${CUR_RESOURCE} DIRECTORY)
        get_filename_component(EXT ${CUR_RESOURCE} LAST_EXT)
        get_filename_component(DIRNAME ${DIR} NAME)

        set(OUT "")

        while (NOT ${DIRNAME} STREQUAL assets)
            get_filename_component(PATH_COMPONENT ${DIR} NAME)
            set(OUT "${PATH_COMPONENT}/${OUT}")
            get_filename_component(DIR ${DIR} DIRECTORY)
            get_filename_component(DIRNAME ${DIR} NAME)
        endwhile ()

        list(FIND SHADER_SOURCE_EXTENSIONS ${EXT} TMP_IS_SHADER)

        if (NOT TMP_IS_SHADER EQUAL -1)
            ProcessShaderResource()
        endif()

    endforeach ()
    add_custom_target(${TargetName}Content ALL DEPENDS ${TMP_RESOURCE_LIST})
endfunction()
