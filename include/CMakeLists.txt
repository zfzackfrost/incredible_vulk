######################################################################
#                     Configuration Header File                      #
######################################################################

configure_file("ivulk/config.hpp.in" "ivulk/config.hpp" @ONLY)

add_library(ivulk_config INTERFACE)
target_include_directories(ivulk_config INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
