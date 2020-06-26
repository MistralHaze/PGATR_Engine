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

    VkQueue _computeQueue;

    //Render pass y pipeline conf
    VkDescriptorSetLayout _descriptorSetLayout;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _computePipeline;

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

    bool alreadyCreatedDSL=false;

  public:
    void run ( );

    void runVulkanCompute ( );

    void cleanup ( );

    //static void onWindowResized ( GLFWwindow* _window, int width, int height );
    static void onWindowResized ( GLFWwindow *window, int width, int height );


    void createInstance ( );

    void setupDebugCallback ( );


    void pickPhysicalDevice ( );

    void createLogicalDevice ( );

    void createDescriptorSetLayout ( );

    void createComputePipeline ( );


    void createCommandPool ( );


    VkFormat findSupportedFormat ( const std::vector < VkFormat > &candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features );


    void copyBufferToImage ( VkBuffer buffer,
                             VkImage image,
                             uint32_t width,
                             uint32_t height );

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

    VkShaderModule createShaderModule ( const std::vector < char > &code );

    bool isDeviceSuitableCompute ( VkPhysicalDevice ldevice );

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
