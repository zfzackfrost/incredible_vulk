######################################################################
#                              Project                               #
######################################################################

project(ex_model_lit)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_model_lit ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_model_lit PUBLIC cxx_std_17)
set_target_properties(ex_model_lit PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_model_lit PUBLIC ivulk)

ConfigureContent(ex_model_lit _assets assets)
