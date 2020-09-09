######################################################################
#                              Project                               #
######################################################################

project(ex_model_spin)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_model_spin ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_model_spin PUBLIC cxx_std_17)
set_target_properties(ex_model_spin PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_model_spin PUBLIC ivulk)

ConfigureContent(ex_model_spin _assets assets)
