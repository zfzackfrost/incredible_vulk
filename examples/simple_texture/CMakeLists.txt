######################################################################
#                              Project                               #
######################################################################

project(ex_simple_texture)

######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_simple_texture ${IVULK_SOURCES})

# ============== Configure Target ============== #

target_compile_features(ex_simple_texture PUBLIC cxx_std_17)
set_target_properties(ex_simple_texture PROPERTIES CXX_EXTENSIONS OFF)

# ================ Dependencies ================ #

target_link_libraries(ex_simple_texture PUBLIC ivulk)

ConfigureContent(ex_simple_texture _assets assets)
