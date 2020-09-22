######################################################################
#                        Run Doxygen on Build                        #
######################################################################

# ========== Find Doxygen executable =========== #

find_package(Doxygen REQUIRED)

# ========== Find public header files ========== #

file(GLOB_RECURSE IVULK_PUBLIC_HEADERS "${IncredibleVulk_SOURCE_DIR}/include")

# ============ Doxygen Configuation ============ #

# Some helper variables
set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(DOXYGEN_INPUT_DIR "${IncredibleVulk_SOURCE_DIR}/include")
set(DOXYGEN_INDEX_XML "${DOXYGEN_OUTPUT_DIR}/xml/index.xml")
set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

# Configure Doxyfile
configure_file(Doxyfile.in Doxyfile @ONLY)

# ===== Ensure the output directory exists ===== #

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# =========== Doxygen Command/Target =========== #

# Only regenerate Doxygen when:
#
# * Doxyfile has changed
# * Public headers have changed
#
add_custom_command(
    OUTPUT ${DOXYGEN_INDEX_XML}
    DEPENDS ${IVULK_PUBLIC_HEADERS}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    MAIN_DEPENDENCY ${DOXYFILE_OUT}
    ${DOXYFILE_IN}
    COMMENT "Generating documentation with Doxygen..."
    VERBATIM
)

# Custom target for running Doxygen
add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_XML})

######################################################################
#                     Run sphinx-build on Build                      #
######################################################################

find_package(Sphinx REQUIRED)

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

file(GLOB_RECURSE SPHINX_RST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.rst")

# Only regenerate Sphinx when:
#
# * Doxygen has rerun
# * Our doc files have been updated
# * The Sphinx config has been updated
#
add_custom_command(
    OUTPUT ${SPHINX_INDEX_FILE}
    COMMAND
        ${SPHINX_EXECUTABLE} -b html
        # Tell Breathe where to find the Doxygen output
        -Dbreathe_projects.IncredibleVulk=${DOXYGEN_OUTPUT_DIR}/xml
        ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        # Sphinx docs files to track
        ${SPHINX_RST_FILES}
        ${DOXYGEN_INDEX_XML}
    MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
    COMMENT "Generating documentation with Sphinx"
)

# Custom target for running Sphinx
add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
