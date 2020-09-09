######################################################################
#                         STB Library Target                         #
######################################################################

add_library(stb INTERFACE IMPORTED)

######################################################################
#                          Configure Target                          #
######################################################################

set(IVULK_STB_ROOT_DIR "${IVULK_EXTERN_DIR}/stb")
set(IVULK_STB_INCLUDE_DIR "${IVULK_STB_ROOT_DIR}")

target_include_directories(stb INTERFACE "${IVULK_STB_INCLUDE_DIR}")
