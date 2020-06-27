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

#include "vulkanApp.h"

const std::vector < const char* > validationLayers = {
  "VK_LAYER_KHRONOS_validation",
//  "VK_LAYER_LUNARG_standard_validation"
};

//Not used here
const std::vector < const char* > deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const unsigned int numOfNumbersToOrder = 2097152; //2^21
const VkDeviceSize bufferSize = sizeof(float) * numOfNumbersToOrder;
const unsigned int workGroupSize = 32;

void vulkanApp::run ( )
{
  std::cout << "Running a Headless Vulkan Compute " << std::endl;
  runVulkanCompute ( );
  cleanup ( );
}


void vulkanApp::runVulkanCompute ( )
{
  //Init
  createInstance ( );
  std::cout << "instance created correctly" << std::endl;

  setupDebugCallback ( );
  std::cout << "callbacks created correctly" << std::endl;

  pickPhysicalDevice ( );
  std::cout << "physDevice picked correctly" << std::endl;

  createLogicalDevice ( );
  std::cout << "(logical)Device created correctly" << std::endl;

  //Pipeline
  createComputePipeline ( );
  std::cout << "pipeline created correctly" << std::endl;

  AllocateBuffer();
  std::cout << "memory correctly allocated" << std::endl;


  createDescriptorPool ( );
  createDescriptorSet ( );
    std::cout << "descriptors created correctly" << std::endl;


  //Commands
  createCommandPool ( );
  createCommandBuffers ( );
  std::cout << "commands created correctly" << std::endl;

  createSemaphores ( );
  std::cout << "semaphores created correctly" << std::endl;

}

//1)Creación de la instancia de la aplicación
void vulkanApp::createInstance ( )
{
  if ( enableValidationLayers && !checkValidationLayerSupport ( ))
  {
    throw std::runtime_error (
      "Validation layers requested, but not available!" );
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan Compute App";
  appInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
  appInfo.pEngineName = "SurfacelessEngine";
  appInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo; //Aqui le pasamos el appInfo!!!

  //VK_EXT_DEBUG_REPORT_EXTENSION_NAME es una extension y se mete al instance aqui
  auto extensions = getRequiredExtensions ( );
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size ( ));
  createInfo.ppEnabledExtensionNames = extensions.data ( );

  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size ( ));
    createInfo.ppEnabledLayerNames = validationLayers.data ( );
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  //La instancia la creamos aqui! Ojo que la guardamos en _instance
  if ( vkCreateInstance ( &createInfo, nullptr, &_instance )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "Failed to create Vulkan Instance!" );
  }
}

//2) Debug
void vulkanApp::setupDebugCallback ( )
{
  if ( !enableValidationLayers )
    return;

  VkDebugReportCallbackCreateInfoEXT createInfo = {};
  createInfo.sType =
    VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags =
    VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  createInfo.pfnCallback = debugCallback;

  if ( CreateDebugReportCallbackEXT ( _instance,
                                      &createInfo,
                                      nullptr,
                                      &_callback ) != VK_SUCCESS )
  {
    throw std::runtime_error ( "Failed to set up debug Callback!" );
  }
}

//3) Coger un dispositivo físico compatible/preferible
void vulkanApp::pickPhysicalDevice ( )
{
  //Ver que dispositivos vulkan capable tenemos
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices ( _instance, &deviceCount, nullptr );

  //Si no hay ninguno finalizamos
  if ( deviceCount == 0 )
  {
    throw std::runtime_error ( "Failed to find Vulkan support on the System!" );
  }

  std::vector < VkPhysicalDevice > devices ( deviceCount );
  vkEnumeratePhysicalDevices ( _instance, &deviceCount, devices.data ( ));

  for ( const auto& ldevice : devices )
  {
    if ( isDeviceSuitableCompute ( ldevice ))
    {
      //Recuerda! Actualmente estamos cogiendo el primero
      _physicalDevice = ldevice;
      break;
    }
  }

  if ( _physicalDevice == VK_NULL_HANDLE )
  {
    throw std::runtime_error ( "Failed to find a suitable device!" );
  }
}

//4)Create Logical _device
void vulkanApp::createLogicalDevice ( )
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies ( _physicalDevice );

  std::vector < VkDeviceQueueCreateInfo > queueCreateInfos;

  float queuePriority = 1.0f;

  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = queueFamilyIndices._computeFamily;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;
  queueCreateInfos.push_back ( queueCreateInfo );

  VkPhysicalDeviceFeatures deviceFeatures = {};

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  //Cuantas colas va a tener. En este caso 1
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size ( ));
  createInfo.pQueueCreateInfos = queueCreateInfos.data ( );
  createInfo.pEnabledFeatures = &deviceFeatures;
  //El swapchain iria aqui como extension. Como no lo usamos no pongo nada 
  createInfo.enabledExtensionCount = 0;
  createInfo.ppEnabledExtensionNames = nullptr;

  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size ( ));
    createInfo.ppEnabledLayerNames = validationLayers.data ( );
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  if ( vkCreateDevice ( _physicalDevice, &createInfo, nullptr, &_device )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create logical _device!" );
  }

  vkGetDeviceQueue ( _device, queueFamilyIndices._computeFamily, 0, &_computeQueue );
}

//8 Creación de los descriptor Sets
void vulkanApp::createDescriptorSetLayout ( )
{
  VkDescriptorSetLayoutBinding uboLayoutBinding = {};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  //TODO: Uniforms go here.

  std::array < VkDescriptorSetLayoutBinding, 1 > bindings = { uboLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size ( ));
  layoutInfo.pBindings = bindings.data ( );

  if ( vkCreateDescriptorSetLayout ( _device,
                                     &layoutInfo,
                                     nullptr,
                                     &_descriptorSetLayout )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create descriptor set layout!" );
  }
}

//5) Creación del pipeline 
void vulkanApp::createComputePipeline ( )
{
  //Ya están los shaders compilados
  std::vector < char > computeShaderCode = readFile ( "./content/vk_shaders/compute/sortingShader.spv" );

  VkShaderModule computeShaderModule = createShaderModule ( computeShaderCode );

  //Generacion y configuracion de cada una de las stages del pipeline
  VkPipelineShaderStageCreateInfo compShaderStageInfo = {};
  compShaderStageInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  compShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  compShaderStageInfo.module = computeShaderModule;
  compShaderStageInfo.pName = "main";

  //Layout -> Manejo de los uniforms de los shaders
  if (!alreadyCreatedDSL)
  {
    createDescriptorSetLayout ( );
    alreadyCreatedDSL=true;
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;

  if ( vkCreatePipelineLayout ( _device,
                                &pipelineLayoutInfo,
                                nullptr,
                                &_pipelineLayout ) != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create pipeline layout!" );
  }


  VkComputePipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = _pipelineLayout;
  pipelineInfo.stage = compShaderStageInfo;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if ( vkCreateComputePipelines ( _device,
                                   VK_NULL_HANDLE,
                                   1,
                                   &pipelineInfo,
                                   nullptr,
                                   &_computePipeline ) != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create compute pipeline!" );
  }


  vkDestroyShaderModule ( _device, computeShaderModule, nullptr );
}

void vulkanApp::AllocateBuffer()
{

 /* createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory );
    VkBuffer computeDataBuffer;
    VkDeviceMemory computeDataBufferMemory;   */          

    createBuffer(bufferSize,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,  
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   _computeBuffer, _computeBufferMemory);

  //FIXME: 
  /*      should I use this? dunno           
  void* data;
  vkMapMemory ( _device, computeDataBufferMemory, 0, vkBufferSize, 0, &data );

  //memcpy ( data, _indices.data ( ), ( size_t ) vkBufferSize );

  vkUnmapMemory ( _device, computeDataBufferMemory );
  */

}


void vulkanApp::cleanup ( )
{
  vkDestroyDescriptorPool ( _device, _descriptorPool, nullptr );

  vkDestroyDescriptorSetLayout ( _device, _descriptorSetLayout, nullptr );
  vkDestroyBuffer ( _device, _computeBuffer, nullptr );
  vkFreeMemory ( _device, _computeBufferMemory, nullptr );

  vkDestroyDevice ( _device, nullptr );
  DestroyDebugReportCallbackEXT ( _instance, _callback, nullptr );


  //Destruir la intancia al final de todo! -> Ultimo objeto de vullkan a
  //destruir
  vkDestroyInstance ( _instance, nullptr );

  //Destroy glfw _window and events
  glfwDestroyWindow ( _window );
  glfwTerminate ( );
}

void vulkanApp::createCommandPool ( )
{
  //Encontrar los _indices de las colas (gráficas para este ejemplo)
  QueueFamilyIndices
    queueFamilyIndices = findQueueFamilies ( _physicalDevice );

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices._computeFamily;

  if ( vkCreateCommandPool ( _device, &poolInfo, nullptr, &_commandPool )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create graphics command pool!" );
  }
}

VkFormat vulkanApp::findSupportedFormat ( const std::vector < VkFormat >& candidates,
                                          VkImageTiling tiling,
                                          VkFormatFeatureFlags features )
{
  for ( VkFormat format : candidates )
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties ( _physicalDevice,
                                          format,
                                          &props );

    if ( tiling == VK_IMAGE_TILING_LINEAR
      && ( props.linearTilingFeatures & features ) == features )
    {
      return format;
    }
    else if ( tiling == VK_IMAGE_TILING_OPTIMAL
      && ( props.optimalTilingFeatures & features ) == features )
    {
      return format;
    }
  }

  throw std::runtime_error ( "failed to find supported format!" );
}

void vulkanApp::copyBufferToImage ( VkBuffer buffer,
                                    VkImage image,
                                    uint32_t width,
                                    uint32_t height )
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands ( );

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = { 0, 0, 0 };
  region.imageExtent = {
    width,
    height,
    1
  };

  vkCmdCopyBufferToImage ( commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region );

  endSingleTimeCommands ( commandBuffer );
}



void vulkanApp::createDescriptorPool ( )
{
  std::array < VkDescriptorPoolSize, 1 > poolSizes = {};
  //De momento solo guardamos guardamos descriptores de Storage
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[0].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size ( ));
  poolInfo.pPoolSizes = poolSizes.data ( );
  poolInfo.maxSets = 1;

  if (
    vkCreateDescriptorPool ( _device, &poolInfo, nullptr, &_descriptorPool )
      != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create descriptor pool!" );
  }
}

void vulkanApp::createDescriptorSet ( )
{
  VkDescriptorSetLayout layouts[] = { _descriptorSetLayout };

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = _descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;

  //Aqui se genera el descriptor set
  if ( vkAllocateDescriptorSets ( _device, &allocInfo, &_descriptorSet )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to allocate descriptor set!" );
  }

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = _computeBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = /*sizeof ( UniformBufferObject )*/ bufferSize;


  std::array < VkWriteDescriptorSet, 1 > descriptorWrites = {};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = _descriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets ( _device,
                           static_cast<uint32_t>(descriptorWrites
                             .size ( )),
                           descriptorWrites.data ( ),
                           0,
                           nullptr );
}

void vulkanApp::createBuffer ( VkDeviceSize size,
                               VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties,
                               VkBuffer& buffer,
                               VkDeviceMemory& bufferMemory )
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if ( vkCreateBuffer ( _device, &bufferInfo, nullptr, &buffer )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create buffer!" );
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements ( _device, buffer, &memRequirements );

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
    findMemoryType ( memRequirements.memoryTypeBits, properties );

  if ( vkAllocateMemory ( _device, &allocInfo, nullptr, &bufferMemory )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to allocate buffer memory!" );
  }

  vkBindBufferMemory ( _device, buffer, bufferMemory, 0 );
}

VkCommandBuffer vulkanApp::beginSingleTimeCommands ( )
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = _commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers ( _device, &allocInfo, &commandBuffer );

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer ( commandBuffer, &beginInfo );

  return commandBuffer;
}

void vulkanApp::endSingleTimeCommands ( VkCommandBuffer commandBuffer )
{
  vkEndCommandBuffer ( commandBuffer );

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit ( _computeQueue, 1, &submitInfo, VK_NULL_HANDLE );
  vkQueueWaitIdle ( _computeQueue );

  vkFreeCommandBuffers ( _device, _commandPool, 1, &commandBuffer );
}

void vulkanApp::copyBuffer ( VkBuffer srcBuffer,
                             VkBuffer dstBuffer,
                             VkDeviceSize size )
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands ( );

  VkBufferCopy copyRegion = {};
  copyRegion.size = size;
  vkCmdCopyBuffer ( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

  endSingleTimeCommands ( commandBuffer );
}

uint32_t vulkanApp::findMemoryType ( uint32_t typeFilter,
                                     VkMemoryPropertyFlags properties )
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties ( _physicalDevice, &memProperties );

  for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
  {
    if (( typeFilter & ( 1 << i ))
      && ( memProperties.memoryTypes[i].propertyFlags & properties )
        == properties )
    {
      return i;
    }
  }

  throw std::runtime_error ( "failed to find suitable memory type!" );
}

//Creación de los command buffers
void vulkanApp::createCommandBuffers ( )
{
  //Tantos command buffers como framebuffers en el swapchain!!!
  //_commandBuffers.resize ( _swapChainFramebuffers.size ( ));

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = _commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = ( uint32_t ) _commandBuffers.size ( );

  if (
    vkAllocateCommandBuffers ( _device,
                               &allocInfo,
                               _commandBuffers.data ( ))
      != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to allocate command buffers!" );
  }

  for ( size_t i = 0; i < _commandBuffers.size ( ); i++ )
  {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer ( _commandBuffers[i], &beginInfo );


    if ( vkEndCommandBuffer ( _commandBuffers[i] ) != VK_SUCCESS )
    {
      throw std::runtime_error ( "failed to record command buffer!" );
    }
  }
}

void vulkanApp::createSemaphores ( )
{
  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if ( vkCreateSemaphore ( _device,
                           &semaphoreInfo,
                           nullptr,
                           &_imageAvailableSemaphore ) != VK_SUCCESS ||
    vkCreateSemaphore ( _device,
                        &semaphoreInfo,
                        nullptr,
                        &_renderFinishedSemaphore ) != VK_SUCCESS )
  {

    throw std::runtime_error ( "failed to create semaphores!" );
  }
}

void vulkanApp::updateUniformBuffer ( )
{
  static auto startTime = std::chrono::high_resolution_clock::now ( );

  auto currentTime = std::chrono::high_resolution_clock::now ( );
  float time = std::chrono::duration_cast < std::chrono::milliseconds > (
    currentTime - startTime ).count ( )/1000.0f;

  UniformBufferObject ubo = {};
  ubo._model = glm::rotate ( glm::mat4 ( 1.0f ),
                            time*glm::radians ( 90.0f ),
                            glm::vec3 ( 0.0f, 0.0f, 1.0f ));
  ubo._view = glm::lookAt ( glm::vec3 ( 2.0f, 2.0f, 2.0f ),
                           glm::vec3 ( 0.0f, 0.0f, 0.0f ),
                           glm::vec3 ( 0.0f, 0.0f, 1.0f ));
  /*ubo._proj = glm::perspective ( glm::radians ( 45.0f ),
                                _swapChainExtent.width
                                  /( float ) _swapChainExtent.height,
                                0.1f,
                                10.0f );*/
  ubo._proj[1][1] *= -1; // switching from OGL to Vulkan sys. coord.

  void* data;
  vkMapMemory ( _device,
                _computeBufferMemory,
                0,
                sizeof ( ubo ),
                0,
                &data );
  memcpy ( data, &ubo, sizeof ( ubo ));
  vkUnmapMemory ( _device, _computeBufferMemory );
}

//Generacion de los shader modules a pasarle al pipeline
VkShaderModule vulkanApp::createShaderModule ( const std::vector < char >& code )
{
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size ( );
  createInfo.pCode = (const uint32_t*)( code.data ( ) );

  VkShaderModule shaderModule;
  if (
    vkCreateShaderModule ( _device, &createInfo, nullptr, &shaderModule )
      != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create shader module!" );
  }

  return shaderModule;
}


bool vulkanApp::isDeviceSuitableCompute ( VkPhysicalDevice ldevice )
{
  bool extensionsSupported = checkDeviceExtensionSupport ( ldevice );
  if ( extensionsSupported )
  {
    //?
  }
  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures ( ldevice, &supportedFeatures );

  //Como solo estamos trabajando con compute no hay que revisar si hay cola de 
  //presentacion o de graficos. Lo que si que habria que revisar es el tamaño 
  //maximo de workGroups o de invocaciones a shaders. Tambien podriamos mirar 
  //si no nos vamos a pasar con el tamaño del buffer
  //De momento como la aplicacion no es pesada simplemente devolveremos el primero. 
  return true;
}


bool vulkanApp::checkDeviceExtensionSupport ( VkPhysicalDevice ldevice )
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties ( ldevice,
                                         nullptr,
                                         &extensionCount,
                                         nullptr );

  std::vector < VkExtensionProperties >
    availableExtensions ( extensionCount );
  vkEnumerateDeviceExtensionProperties ( ldevice,
                                         nullptr,
                                         &extensionCount,
                                         availableExtensions.data ( ));


  //Esta parte hay que revisarla un poco ####
  std::set < std::string > requiredExtensions
    ( deviceExtensions.begin ( ), deviceExtensions.end ( ));

  for ( const auto& extension : availableExtensions )
  {
    requiredExtensions.erase ( extension.extensionName );
  }

  return requiredExtensions.empty ( );
}

QueueFamilyIndices vulkanApp::findQueueFamilies ( VkPhysicalDevice ldevice )
{
  //Indices de las colas
  QueueFamilyIndices queueFamilyIndices;

  //Numero de colas
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties ( ldevice,
                                             &queueFamilyCount,
                                             nullptr );

  //Informacion de todas las colas
  std::vector < VkQueueFamilyProperties >
    queueFamilies ( queueFamilyCount );
  vkGetPhysicalDeviceQueueFamilyProperties ( ldevice,
                                             &queueFamilyCount,
                                             queueFamilies.data ( ));

  int i = 0;
  for ( const auto& queueFamily : queueFamilies )
  {
    //Es una cola de computo
    if ( queueFamily.queueCount > 0
      && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT )
    {
      queueFamilyIndices._computeFamily = i;
    }

    if ( queueFamilyIndices.isComplete ( ))
    {
      break;
    }

    i++;
  }

  return queueFamilyIndices;
}


std::vector < const char* > vulkanApp::getRequiredExtensions ( )
{
  std::vector < const char* > extensions;

  if ( enableValidationLayers )
  {
    extensions.push_back ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
  }

  return extensions;
}

bool vulkanApp::checkValidationLayerSupport ( )
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

  std::vector < VkLayerProperties > availableLayers ( layerCount );
  vkEnumerateInstanceLayerProperties ( &layerCount,
                                       availableLayers.data ( ));

  for ( const char* layerName : validationLayers )
  {
    for ( const auto& layerProperties : availableLayers )
    {
      if ( strcmp ( layerName, layerProperties.layerName ) == 0 )
      {
        return true;
      }
    }
  }

  return false;
}

std::vector < char > vulkanApp::readFile ( const std::string& filename )
{
  std::ifstream file ( filename, std::ios::ate | std::ios::binary );

  if ( !file.is_open ( ))
  {
    throw std::runtime_error ( "failed to open file!" );
  }

  //Empezamos por el final para tener la longitud del fichero
  size_t fileSize = ( size_t ) file.tellg ( );
  std::vector < char > buffer ( fileSize );

  //Reposicionamos la principio y leemos del tiron
  file.seekg ( 0 );
  file.read ( buffer.data ( ), fileSize );

  file.close ( );

  return buffer;
}


VKAPI_ATTR VkBool32
VKAPI_CALL vulkanApp::debugCallback ( VkDebugReportFlagsEXT flags,
                                      VkDebugReportObjectTypeEXT objType,
                                      uint64_t obj,
                                      size_t location,
                                      int32_t code,
                                      const char* layerPrefix,
                                      const char* msg,
                                      void* userData )
{
  std::cerr << "validation layer: " << msg << std::endl;

  //TODO: Esta reconversion porque?
  ( void ) flags;
  ( void ) objType;
  ( void ) obj;
  ( void ) location;
  ( void ) code;
  ( void ) layerPrefix;
  ( void ) userData;

  return VK_FALSE;
}
