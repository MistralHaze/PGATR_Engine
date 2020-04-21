source_group(\\ FILES CMakeLists.txt)

set(VKNGINE_PUBLIC_HEADERS vulkanApp.h
)

set(VKNGINE_HEADERS )

set(VKNGINE_SOURCES    vkDebuger.hpp
                        vkBaseTypes.hpp
                        vkHelper.hpp
                        vulkanApp.cpp )

set(VKNGINE_LINK_LIBRARIES  ${FREEIMAGE_LIBRARIES}
                             ${ASSIMP_LIBRARIES}
)

common_library( VKNgine )
