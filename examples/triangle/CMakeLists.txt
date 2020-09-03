######################################################################
#                              Project                               #
######################################################################

project(ex_triangle)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_triangle ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_triangle PUBLIC cxx_std_17)
set_target_properties(ex_triangle PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_triangle PUBLIC ivulk)

ConfigureContent(ex_triangle ${PROJECT_SOURCE_DIR}/assets/shaders)
