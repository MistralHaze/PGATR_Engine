# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
#   VKNgine
#   2019 (c) Juan Pedro Brito Méndez
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

cmake_minimum_required( VERSION 3.1 FATAL_ERROR )
project( VKNgine VERSION 0.0.1 )
set( VKNgine_VERSION_ABI 1)

# Disable in source building
if( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
  message( FATAL "no in source building allowed." )
endif()

list( APPEND CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common )

# Default build type is Debug
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Debug )
endif( NOT CMAKE_BUILD_TYPE )


if ( CMAKE_BUILD_TYPE MATCHES "Debug" )
  add_definitions( -DDEBUG )
endif( )

include(GitExternal)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES" )

find_library(VULKAN_LIBRARY NAMES vulkan vulkan-1)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

set(IMGUI_DIR libs/imgui)
include_directories( ${IMGUI_DIR} )

set( VKNGINE_DESCRIPTION "Simple tool to test different multithreaded technologies." )
set( VKNGINE_LICENSE Apache 2 )
set( VKNGINE_MAINTAINER "Juan P. Brito <juanpedro.brito@upm.es>" )

include( Common )

#common_find_package(Eigen3 REQUIRED SYSTEM)
common_find_package(assimp SYSTEM REQUIRED)
common_find_package(FreeImage  REQUIRED)

# Vulkan
common_find_package(GLFW3 REQUIRED SYSTEM)
common_find_package(XCB SYSTEM  REQUIRED)

list(APPEND VKNGINE_DEPENDENT_LIBRARIES assimp FreeImage)

if(FREEIMAGE_FOUND)
  list(APPEND VKNGINE_DEPENDENT_LIBRARIES FreeImage)
endif()

common_find_package_post( )

# imgui lib
set(IMGUI_DIR libs/imgui)
set (   IMGUI_HEADERS
        ${IMGUI_DIR}/imconfig.h
        ${IMGUI_DIR}/imgui.h
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imstb_rectpack.h
        ${IMGUI_DIR}/imstb_textedit.h
        ${IMGUI_DIR}/imstb_truetype.h
        ${IMGUI_DIR}/examples/imgui_impl_opengl2.h
        ${IMGUI_DIR}/examples/imgui_impl_opengl3.h
        ${IMGUI_DIR}/examples/imgui_impl_glut.h
        ${IMGUI_DIR}/examples/imgui_impl_glfw.h
        ${IMGUI_DIR}/examples/imgui_impl_vulkan.h
        )

set (   IMGUI_SRC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/examples/imgui_impl_opengl2.cpp
        ${IMGUI_DIR}/examples/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/examples/imgui_impl_glut.cpp
        ${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/examples/imgui_impl_vulkan.cpp
        VKNgine/vulkanApp.cpp VKNgine/vulkanApp.h)

add_library(imgui ${IMGUI_SRC} ${IMGUI_HEADERS})
include_directories(${IMGUI_DIR} libs/imgui)

add_subdirectory( VKNgine )
add_subdirectory( examples )

file(GLOB CONTENT "content/*")
file(COPY ${CONTENT} DESTINATION "${CMAKE_BINARY_DIR}/bin/content")
