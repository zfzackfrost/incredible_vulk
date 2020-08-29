######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/main.cpp")

######################################################################
#                         Executable Target                          #
######################################################################

add_executable(ex_just_vulkan ${IVULK_SOURCES})

# ============= Configure Target ============== #

target_compile_features(ex_just_vulkan PUBLIC cxx_std_17)
set_target_properties(ex_just_vulkan PROPERTIES CXX_EXTENSIONS OFF)

# =============== Dependencies ================ #

target_link_libraries(ex_just_vulkan PUBLIC ivulk)

ConfigureContent(ex_just_vulkan ${CMAKE_CURRENT_LIST_DIR}/assets/shaders)
