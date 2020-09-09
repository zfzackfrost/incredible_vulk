include(FetchContent)

set(IVULK_BOOST_BINARY_LIBS "")
list(APPEND IVULK_BOOST_BINARY_LIBS "filesystem")


function(DownloadIvulkBoost)
    FetchContent_Declare(
        ivulkboost
        URL      https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz
        URL_HASH SHA256=afff36d392885120bcac079148c177d1f6f7730ec3d47233aa51b0afa4db94a5
    )
    FetchContent_GetProperties(ivulkboost)
    if(NOT ivulkboost_POPULATED)
        FetchContent_Populate(ivulkboost)
        set(Boost_SOURCE_DIR ${ivulkboost_SOURCE_DIR})
        message("Boost Source Dir: ${Boost_SOURCE_DIR}")

        set(BOOST_BUILD_FLAGS "")
        foreach(Lib IN LISTS IVULK_BOOST_BINARY_LIBS)
            set(BOOST_BUILD_FLAGS "${BOOST_BUILD_FLAGS} --with-${Lib}")
        endforeach()

        execute_process(
            COMMAND ${ivulkboost_SOURCE_DIR}/bootstrap.sh
            WORKING_DIRECTORY "${Boost_SOURCE_DIR}")
        execute_process(
            COMMAND bash -c "${ivulkboost_SOURCE_DIR}/b2 ${BOOST_BUILD_FLAGS} stage"
            WORKING_DIRECTORY ${Boost_SOURCE_DIR})

        set(BOOST_ROOT ${Boost_SOURCE_DIR})
        set(BOOST_INCLUDEDIR ${Boost_SOURCE_DIR})
        set(BOOST_LIBRARYDIR ${Boost_SOURCE_DIR}/stage/lib)
        set(Boost_NO_SYSTEM_PATHS ON)
        set(Boost_USE_STATIC_LIBS FALSE)
        find_package (Boost 1.73 REQUIRED COMPONENTS ${IVULK_BOOST_BINARY_LIBS})
    endif()
endfunction()

option(IVULK_USE_SYSTEM_BOOST "If enabled, use the Boost libraries installed on this system. Otherwise force download." OFF)
if (IVULK_USE_SYSTEM_BOOST)
    find_package(Boost 1.73 COMPONENTS ${IVULK_BOOST_BINARY_LIBS})

    if (NOT Boost_FOUND AND IVULK_DOWNLOAD_MISSING)
        DownloadIvulkBoost()
    endif ()

    message(STATUS "Boost version: ${Boost_VERSION}")
else()
    DownloadIvulkBoost()
endif ()

add_library(BoostSupport INTERFACE)
add_library(Boost::support ALIAS BoostSupport)

if (UNIX)
    target_link_libraries(BoostSupport INTERFACE pthread)
endif()
