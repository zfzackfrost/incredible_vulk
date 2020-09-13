######################################################################
#                              Sources                               #
######################################################################

set(IVULK_SOURCES "")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/app.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/app__drawing.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/app__graphics_pipeline.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/app__swap_chain.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/app__sync.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/buffer.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/command_buffer.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/event.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/graphics_pipeline.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/image.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/sampler.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/core/vma.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/render/scene.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/render/renderable_instance.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/render/model/static_model.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/utils/format.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/utils/fs.cpp")
list(APPEND IVULK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/ivulk/utils/messages.cpp")

list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/app.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/buffer.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/command_buffer.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/event.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/graphics_pipeline.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/image.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/sampler.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/core/vma.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/render/model/base.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/render/model/static_model.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/utils/format.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/utils/fs.hpp")
list(APPEND IVULK_SOURCES "${PROJECT_SOURCE_DIR}/include/ivulk/utils/messages.hpp")

######################################################################
#                           Library Target                           #
######################################################################

add_library(ivulk SHARED ${IVULK_SOURCES})

# ============= Configure Target ============== #

target_compile_features(ivulk PUBLIC cxx_std_17)
set_target_properties(ivulk PROPERTIES CXX_EXTENSIONS OFF)

# =============== Dependencies ================ #

target_include_directories(ivulk PUBLIC "${IncredibleVulk_SOURCE_DIR}/include")
target_link_libraries(ivulk PUBLIC ivulk_config)
target_link_libraries(ivulk PUBLIC SDL2-static SDL2main)
target_link_libraries(ivulk PUBLIC Vulkan::Vulkan)
target_link_libraries(ivulk PUBLIC glm)
target_link_libraries(ivulk PUBLIC Vma)
target_link_libraries(ivulk PUBLIC stb)
target_link_libraries(ivulk PUBLIC assimp)
target_link_libraries(ivulk PUBLIC Boost::headers Boost::filesystem)
