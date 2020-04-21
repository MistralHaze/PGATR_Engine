include_directories(${PROJECT_BINARY_DIR}/include
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

set(VK_GLFW_IMGUI_HEADERS)
set(VK_GLFW_IMGUI_SOURCES vk_glfw_imgui.cpp )
set(VK_GLFW_IMGUI_LINK_LIBRARIES
        imgui
        ${GLFW3_LIBRARY}
        #${GLEW_LIBRARY}
        ${VULKAN_LIBRARY}
        )
common_application(vk_glfw_imgui)

#VK-glfw test
set(VK_GLFW_HEADERS)
set(VK_GLFW_SOURCES vk_glfw.cpp )
set(VK_GLFW_LINK_LIBRARIES
        VKNgine
        ${GLFW3_LIBRARY}
        #${GLEW_LIBRARY}
        ${VULKAN_LIBRARY}
        )
common_application(vk_glfw)


