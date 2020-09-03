######################################################################
#                        Add Imported Target                         #
######################################################################

set(GLM_INCLUDE_DIR "${IncredibleVulk_SOURCE_DIR}/extern/glm")

add_library(glm INTERFACE IMPORTED)
target_include_directories(glm INTERFACE "${GLM_INCLUDE_DIR}")
