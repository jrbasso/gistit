cmake_minimum_required(VERSION 2.8)

project(gistit)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(CURL REQUIRED)
find_package(Jansson REQUIRED)

include_directories(src)
include_directories(${JANSSON_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

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
