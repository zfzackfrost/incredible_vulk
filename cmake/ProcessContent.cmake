set(IVULK_SHADER_SOURCE_EXTENSIONS "")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".frag")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".vert")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".tesc")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".tese")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".geom")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".comp")
list(APPEND IVULK_SHADER_SOURCE_EXTENSIONS ".hlsl")

set(IVULK_SHADERLIB_DIR "${IncredibleVulk_SOURCE_DIR}/assets/shaderlib")

option(IVULK_BUILD_CONTENT "Build project content" ON)

macro(ProcessNormalResource)
    set(TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/${AssetSubdir}/${OUT})
    set(OUT "${CMAKE_CURRENT_BINARY_DIR}/${AssetSubdir}/${OUT}/${FNAME}")

    add_custom_command(
            COMMENT "Moving updated resource file '${FNAME}'"
            OUTPUT ${OUT}
            DEPENDS ${CUR_RESOURCE}
            COMMAND ${CMAKE_COMMAND} -E make_directory
                ${TMP_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CUR_RESOURCE}
                ${OUT}
    )

    list(APPEND TMP_RESOURCE_LIST ${CUR_RESOURCE} ${OUT})
endmacro()

macro(ProcessShaderResource)
    set(TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/${AssetSubdir}/${OUT})
    get_filename_component(INPUT_DIR ${CUR_RESOURCE} DIRECTORY)
    set(OUT "${PROJECT_BINARY_DIR}/${AssetSubdir}/${OUT}/${FNAME}.spv")
    add_custom_command(
            COMMENT "Compiling shader file '${FNAME}'"
            OUTPUT ${OUT}
            DEPENDS ${CUR_RESOURCE}
            COMMAND ${CMAKE_COMMAND} -E make_directory
                ${TMP_DIR}
            COMMAND ${GLSLC_EXECUTABLE} "${CUR_RESOURCE}" -o "${OUT}" -I "${IVULK_SHADERLIB_DIR}" -I "${INPUT_DIR}"
    )

    list(APPEND TMP_RESOURCE_LIST ${CUR_RESOURCE} ${OUT})
endmacro()

# Arguments:
#   TargetName: The name of the original target to process content for
#   TargetSuffix: The suffix to add to `TargetName` when creating the content target
#   AssetSubdir: The current source subdirectory that contains the assets
function(ConfigureContent TargetName TargetSuffix AssetSubdir)
    if (IVULK_BUILD_CONTENT)
        set(TMP_CONTENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${AssetSubdir})

        file(GLOB_RECURSE PROJ_CONTENT "${TMP_CONTENT_DIR}/*.*")
        set(TMP_RESOURCE_LIST "")

        foreach (CUR_RESOURCE ${PROJ_CONTENT})
            get_filename_component(FNAME ${CUR_RESOURCE} NAME)
            get_filename_component(DIR ${CUR_RESOURCE} DIRECTORY)
            get_filename_component(EXT ${CUR_RESOURCE} LAST_EXT)
            get_filename_component(DIRNAME ${DIR} NAME)

            set(OUT "")

            while (NOT ${DIRNAME} STREQUAL ${AssetSubdir})
                get_filename_component(PATH_COMPONENT ${DIR} NAME)
                set(OUT "${PATH_COMPONENT}/${OUT}")
                get_filename_component(DIR ${DIR} DIRECTORY)
                get_filename_component(DIRNAME ${DIR} NAME)
            endwhile ()

            list(FIND IVULK_SHADER_SOURCE_EXTENSIONS ${EXT} TMP_IS_SHADER)

            if (TMP_IS_SHADER EQUAL -1)
                ProcessNormalResource()
            else()
                ProcessShaderResource()
            endif()

        endforeach ()
        add_custom_target(${TargetName}${TargetSuffix} ALL DEPENDS ${TMP_RESOURCE_LIST})
        add_dependencies(${TargetName} ${TargetName}${TargetSuffix})
    else()
        add_custom_target(${TargetName} ALL)
    endif()
endfunction()
