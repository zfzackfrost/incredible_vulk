######################################################################
#                              Project                               #
######################################################################

project(ex_cube_spin)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_cube_spin ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_cube_spin PUBLIC cxx_std_17)
set_target_properties(ex_cube_spin PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_cube_spin PUBLIC ivulk)

ConfigureContent(ex_cube_spin ${PROJECT_SOURCE_DIR}/assets/shaders)
