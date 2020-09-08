######################################################################
#                              Project                               #
######################################################################

project(ex_squares_3d)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_squares_3d ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_squares_3d PUBLIC cxx_std_17)
set_target_properties(ex_squares_3d PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_squares_3d PUBLIC ivulk)

ConfigureContent(ex_squares_3d _assets assets)
