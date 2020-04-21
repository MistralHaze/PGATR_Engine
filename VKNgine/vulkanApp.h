/*
# December 27 2019,
#
# Juan Pedro Brito Méndez <juanpedro.brito@upm.es>
#
# Copyright (c) 2019, Center for Computational Simulation - Universidad Politécnica de Madrid
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
*/

#ifndef VULKANAPP_H
#define VULKANAPP_H

//Std libs
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <array>
#include <set>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "vkBaseTypes.hpp"
#include "vkDebuger.hpp"
#include "vkHelper.hpp"

class vulkanApp
{

  private:
    //Ventana de renderizado de GLFW
    GLFWwindow *_window;

    //Instancia
    VkInstance _instance;

    //Debug options
    VkDebugReportCallbackEXT _callback;

    //Physical y logical devices
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device;

    //Colas de procesamiento gráfica y de presentacion (pueden estar por separado)
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    //Superficie de renderizado
    VkSurfaceKHR _surface;

    //Toda la configuracion de la swapchain
    VkSwapchainKHR _swapChain;
    //Formato utilizado por las imágenes del swapchain
    VkFormat _swapChainImageFormat;
    //Resolución de las imágenes
    VkExtent2D _swapChainExtent;
    //Vector de imágenes utilizado por la swapchain
    std::vector < VkImage > _swapChainImages;
    //Vector de presentador de imágenes utilizado por la swapchain
    std::vector < VkImageView > _swapChainImageViews;
    //Vector de almacenamiento de imágenes utilizado por la swapchain
    std::vector < VkFramebuffer > _swapChainFramebuffers;


    //Render pass y pipeline conf
    VkRenderPass _renderPass;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _graphicsPipeline;

    //Image eand texture management
    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    VkImage _textureImage;
    VkDeviceMemory _textureImageMemory;
    VkImageView _textureImageView;
    VkSampler _textureSampler;

    //Mallado
    std::vector < Vertex > _vertices;
    std::vector < uint32_t > _indices;
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;

    //UBOS
    VkBuffer _uniformBuffer;
    VkDeviceMemory _uniformBufferMemory;

    //Descriptores
    VkDescriptorPool _descriptorPool;
    VkDescriptorSet _descriptorSet;

    //Command buffers
    VkCommandPool _commandPool;
    std::vector < VkCommandBuffer > _commandBuffers;

    //Sincronización
    //Imagen adquirida y preparada para renderizarse.
    VkSemaphore _imageAvailableSemaphore;

    //Renderizado acabado, ya se puede presentar.
    VkSemaphore _renderFinishedSemaphore;

    bool alreadeCreatedDSL=false;

  public:
    void run ( );

    void initWindow ( );

    void initVulkan ( );

    void mainLoop ( );

    void cleanupSwapChain ( );

    void cleanup ( );

    //static void onWindowResized ( GLFWwindow* _window, int width, int height );
    static void onWindowResized ( GLFWwindow *window, int width, int height );

    void recreateSwapChain ( );

    void createInstance ( );

    void setupDebugCallback ( );

    void createSurface ( );

    void pickPhysicalDevice ( );

    void createLogicalDevice ( );

    void createSwapChain ( );

    void createImageViews ( );

    void createRenderPass ( );

    void createDescriptorSetLayout ( );

    void createGraphicsPipeline ( );

    void createFramebuffers ( );

    void createCommandPool ( );

    void createDepthResources ( );

    VkFormat findSupportedFormat ( const std::vector < VkFormat > &candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features );

    VkFormat findDepthFormat ( );

    bool hasStencilComponent ( VkFormat format );

    void createTextureImage ( );

    void createTextureImageView ( );

    void createTextureSampler ( );

    VkImageView createImageView ( VkImage image,
                                  VkFormat format,
                                  VkImageAspectFlags aspectFlags );

    void createImage ( uint32_t width,
                       uint32_t height,
                       VkFormat format,
                       VkImageTiling tiling,
                       VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkImage &image,
                       VkDeviceMemory &imageMemory );

    void transitionImageLayout ( VkImage image,
                                 VkFormat format,
                                 VkImageLayout oldLayout,
                                 VkImageLayout newLayout );

    void copyBufferToImage ( VkBuffer buffer,
                             VkImage image,
                             uint32_t width,
                             uint32_t height );

    void loadModel ( );

    void createVertexBuffer ( );

    void createIndexBuffer ( );

    void createUniformBuffer ( );

    void createDescriptorPool ( );

    void createDescriptorSet ( );

    void createBuffer ( VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkBuffer &buffer,
                        VkDeviceMemory &bufferMemory );

    VkCommandBuffer beginSingleTimeCommands ( );

    void endSingleTimeCommands ( VkCommandBuffer commandBuffer );

    void copyBuffer ( VkBuffer srcBuffer,
                      VkBuffer dstBuffer,
                      VkDeviceSize size );

    uint32_t findMemoryType ( uint32_t typeFilter,
                              VkMemoryPropertyFlags properties );

    void createCommandBuffers ( );

    void createSemaphores ( );

    void updateUniformBuffer ( );

    void drawFrame ( );

    VkShaderModule createShaderModule ( const std::vector < char > &code );

    VkSurfaceFormatKHR chooseSwapSurfaceFormat ( const std::vector <
      VkSurfaceFormatKHR > &availableFormats );

    VkPresentModeKHR chooseSwapPresentMode ( const std::vector <
      VkPresentModeKHR > availablePresentModes );

    VkExtent2D chooseSwapExtent ( const VkSurfaceCapabilitiesKHR &
    capabilities );

    SwapChainSupportDetails querySwapChainSupport ( VkPhysicalDevice
                                                    ldevice );

    bool isDeviceSuitable ( VkPhysicalDevice ldevice );

    bool checkDeviceExtensionSupport ( VkPhysicalDevice ldevice );

    QueueFamilyIndices findQueueFamilies ( VkPhysicalDevice ldevice );
    std::vector < const char * > getRequiredExtensions ( );

    bool checkValidationLayerSupport ( );

    static std::vector < char > readFile ( const std::string &filename );

    static VKAPI_ATTR VkBool32
    VKAPI_CALL debugCallback ( VkDebugReportFlagsEXT flags,
                               VkDebugReportObjectTypeEXT objType,
                               uint64_t obj,
                               size_t location,
                               int32_t code,
                               const char *layerPrefix,
                               const char *msg,
                               void *userData );
};

#endif //VULKANAPP_H
