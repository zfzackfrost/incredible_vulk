######################################################################
#                              Project                               #
######################################################################

project(ex_rectangle)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_rectangle ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_rectangle PUBLIC cxx_std_17)
set_target_properties(ex_rectangle PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_rectangle PUBLIC ivulk)

ConfigureContent(ex_rectangle ${PROJECT_SOURCE_DIR}/assets/shaders)
