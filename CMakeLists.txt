cmake_minimum_required(VERSION 2.8)

project(gistit)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(CURL REQUIRED)
find_package(Jansson REQUIRED)

include_directories(.)
include_directories(src)
include_directories(${JANSSON_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

set(GISTIT_MAJOR_VERSION 0)
set(GISTIT_MINOR_VERSION 1)
set(GISTIT_PATCH_VERSION 3)
set(GISTIT_VERSION ${GISTIT_MAJOR_VERSION}.${GISTIT_MINOR_VERSION}.${GISTIT_PATCH_VERSION})
set(GISTIT_NAME "Gist It!")

configure_file(src/config.h.in config.h)

set(GISTIT_SOURCES
    src/filelist.c
    src/gistit.c
    src/github.c)

add_executable(gistit ${GISTIT_SOURCES})
target_link_libraries(gistit
    ${JANSSON_LIBRARIES}
    ${CURL_LIBRARIES})

INSTALL(TARGETS gistit
        DESTINATION "bin")
