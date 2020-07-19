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

#include "renderdoc.h"
#include <dlfcn.h>

RENDERDOC_API_1_1_2 *rdoc_api = NULL;

const std::vector < const char* > validationLayers = {
  "VK_LAYER_KHRONOS_validation",
//  "VK_LAYER_LUNARG_standard_validation"
};

//Not used here
const std::vector < const char* > deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#define VK_CHECK_RESULT(f) 																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
        assert(res == VK_SUCCESS);																		\
    }																									\
}

const  float numOfNumbersToOrder = 1024;/*2097152; //2^21*/
// use this or alignas(4)
const VkDeviceSize bufferSize =  sizeof(glm::vec4) * numOfNumbersToOrder;
const unsigned int workGroupSize = 32;

//std::vector<float> computeInput(numOfNumbersToOrder);
//std::vector<float> computeOutput(numOfNumbersToOrder);

std::vector<glm::vec4> computeInput(numOfNumbersToOrder);
std::vector<glm::vec4> computeOutput(numOfNumbersToOrder);

void vulkanApp::run ( )
{
  std::cout << "Running a Headless Vulkan Compute " << std::endl;

  setupRenderdoc();

  startRenderdocRecording();
  runVulkanCompute ( );
  endRenderdocRecording();
  cleanup ( );
}

void vulkanApp::setupRenderdoc()
{ 
  std::cout << "Initializing Renderdoc" << std::endl;

  if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
  {
      pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
      int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
      assert(ret == 1);
      std::cout<<"recording" << std::endl;
  }else
  {
      std::cout<<"Renderdoc could not be set up. Failure in recording" << std::endl;
  }
}

void vulkanApp::startRenderdocRecording()
{
  // To start a frame capture, call StartFrameCapture.
  // You can specify NULL, NULL for the device to capture on if you have only one device and
  // either no windows at all or only one window, and it will capture from that device.
  // See the documentation below for a longer explanation
  if(rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
}

void vulkanApp::endRenderdocRecording()
{
  // stop the capture
  if(rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
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

  createFences();

  //Commands
  createCommandPool ( );
  manageCommandBuffers ( );
  std::cout << "commands executed correctly" << std::endl;

  readOutput();
    std::cout << "finished program" << std::endl;

}

void vulkanApp::createFences()
{
  
  VkFenceCreateInfo fenceCreateInfo = {};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = 0;
  VK_CHECK_RESULT(vkCreateFence(_device, &fenceCreateInfo, NULL, &_fence));
}

void vulkanApp::readOutput()
{

  vkDestroyFence(_device, _fence, NULL);

  // Make device writes visible to the host
  void *mapped;
  vkMapMemory(_device, _computeBufferMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
  VkMappedMemoryRange mappedRange {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;		
  mappedRange.memory = _computeBufferMemory;
  mappedRange.offset = 0;
  mappedRange.size = VK_WHOLE_SIZE;
  vkInvalidateMappedMemoryRanges(_device, 1, &mappedRange);

  // Copy to output
  memcpy(computeOutput.data(), mapped, bufferSize);
  vkUnmapMemory(_device, _computeBufferMemory);

  std::cout << "Entrada "<< computeInput.size()<< std::endl;
  for(unsigned int i=0; i<computeInput.size(); i++)
  {
    std::cout <<computeInput[i].x << " " ;
  }
  std::cout<<std::endl;

  std::cout << "Salida "<< computeOutput.size()<< std::endl;
  for(unsigned int i=0; i<computeOutput.size(); i++)
  {
    std::cout  <<computeOutput[i].x << " " ;
  }
  std::cout << std::endl ;

  //vkUnmapMemory(_device, _computeBufferMemory);
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
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
/*
  VkDescriptorSetLayoutBinding uniformLayoutBinding = {};
  uniformLayoutBinding.binding = 1;
  uniformLayoutBinding.descriptorCount = 1;
  uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniformLayoutBinding.pImmutableSamplers = nullptr;
  uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;*/


  std::array < VkDescriptorSetLayoutBinding, 1 > bindings = { uboLayoutBinding/*, uniformLayoutBinding*/};

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
  std::vector < char > computeShaderCode = readFile ( "./content/vk_shaders/compute/passThrough.spv" );

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
////////////////////
//staging buffer
////////////////////////

 	//uint32_t n = 0;
	//std::generate(computeInput.begin(), computeInput.end(), [&n] { return rand()/10000000; });
  for(int i=0; i<numOfNumbersToOrder; i++)
  {
    computeInput[i].x =rand()/10000000;
    computeInput[i].y = computeInput[i].z =0;

  }

 createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 _stagingBuffer,
                 _stagingBufferMemory );
                 

  void* mappedData;
  vkMapMemory ( _device, _stagingBufferMemory, 0, bufferSize, 0, &mappedData );
  memcpy ( mappedData, computeInput.data ( ), ( size_t ) bufferSize );

  // Flush writes to host visible buffer
  VkMappedMemoryRange mappedRange {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = _stagingBufferMemory;
  mappedRange.offset = 0;
  mappedRange.size = VK_WHOLE_SIZE;
  vkFlushMappedMemoryRanges(_device, 1, &mappedRange);
  vkUnmapMemory(_device, _stagingBufferMemory);

   
  createBuffer(bufferSize,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT,  
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                   _computeBuffer,_computeBufferMemory );


////////////////////
//Uniforms
////////////////////////
  VkDeviceSize uniformBufferSize = sizeof ( UniformBufferObject );
  createBuffer ( uniformBufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 _uniformBuffer,
                 _uniformBufferMemory );

  for(unsigned int i=0; i<numOfNumbersToOrder; i++)
  {
    std::cout << computeInput[i].x << " " ;
  }
  std::cout << std::endl ;


}


void vulkanApp::cleanup ( )
{
  vkDestroyDescriptorPool ( _device, _descriptorPool, nullptr );
  vkDestroyDescriptorSetLayout ( _device, _descriptorSetLayout, nullptr );

  vkDestroyBuffer ( _device, _computeBuffer, nullptr );
  vkFreeMemory ( _device, _computeBufferMemory, nullptr );

  vkDestroyBuffer ( _device, _stagingBuffer, nullptr );
  vkFreeMemory ( _device, _stagingBufferMemory, nullptr );

  vkDestroyBuffer ( _device, _uniformBuffer, nullptr );
  vkFreeMemory ( _device, _uniformBufferMemory, nullptr );

  vkDestroyCommandPool(_device, _commandPool, nullptr);

  vkDestroyPipeline(_device, _computePipeline, nullptr);
  vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);

  vkDestroyDevice ( _device, nullptr );
  DestroyDebugReportCallbackEXT ( _instance, _callback, nullptr );

  //Destruir la intancia al final de todo! -> Ultimo objeto de vullkan a
  //destruir
  vkDestroyInstance ( _instance, nullptr );

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



void vulkanApp::createDescriptorPool ( )
{
  std::array < VkDescriptorPoolSize, 1> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[0].descriptorCount = 1;
  /*poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[1].descriptorCount = 1;*/

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
  bufferInfo.range = bufferSize;
/*VkDescriptorBufferInfo uniformInfo = {};
  bufferInfo.buffer = _uniformBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = sizeof ( UniformBufferObject );*/


  std::array < VkWriteDescriptorSet, 1 > descriptorWrites = {};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = _descriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pBufferInfo = &bufferInfo;
/*
  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = _descriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].dstArrayElement = 0;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].pBufferInfo = &uniformInfo;*/



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

void vulkanApp::endSingleTimeCommands ( VkCommandBuffer commandBuffer, VkFence fence )
{
  //vkEndCommandBuffer ( commandBuffer );

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit ( _computeQueue, 1, &submitInfo,  fence );
  vkQueueWaitIdle ( _computeQueue );
  VK_CHECK_RESULT(vkWaitForFences(_device, 1, &_fence, VK_TRUE, 10000000));


  vkFreeCommandBuffers ( _device, _commandPool, 1, &commandBuffer );

  vkResetFences(_device, 1, &_fence);
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
void vulkanApp::manageCommandBuffers ( )
{
  VkCommandBufferAllocateInfo allocInfoCopy = {};
  allocInfoCopy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfoCopy.commandPool = _commandPool;
  allocInfoCopy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfoCopy.commandBufferCount = 1;

  VkCommandBufferAllocateInfo allocInfoCompute = {};
  allocInfoCompute.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfoCompute.commandPool = _commandPool;
  allocInfoCompute.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfoCompute.commandBufferCount = 1;


  if (vkAllocateCommandBuffers ( _device,
                               &allocInfoCompute,
                               &_commandBufferCompute) //_commandBuffer
      != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to allocate command buffers!" );
  }

    if (vkAllocateCommandBuffers ( _device,
                               &allocInfoCopy,
                               &_commandBufferCopy) //_commandBuffer
      != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to allocate command buffers!" );
  }



  ///////Copy

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer ( _commandBufferCopy, &beginInfo );


    //copy to staging buffer
    VkBufferCopy copyRegion = {};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(_commandBufferCopy, _stagingBuffer, _computeBuffer, 1, &copyRegion);
    if ( vkEndCommandBuffer ( _commandBufferCopy ) != VK_SUCCESS )
    {
      throw std::runtime_error ( "failed to record command buffer!" );
    }
  
    endSingleTimeCommands(_commandBufferCopy, _fence);

  //////Compute

    /*VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;*/

    vkBeginCommandBuffer ( _commandBufferCompute, &beginInfo );


    vkCmdBindPipeline(_commandBufferCompute, VK_PIPELINE_BIND_POINT_COMPUTE, _computePipeline);
    // 0:firstSet is the set number of the first descriptor set to be bound.
    // 1:descriptorSetCount is the number of elements in the pDescriptorSets array.
    // 0 :dynamicOffsetCount is the number of dynamic offsets in the pDynamicOffsets array.
    vkCmdBindDescriptorSets(_commandBufferCompute, VK_PIPELINE_BIND_POINT_COMPUTE, _pipelineLayout, 0,1,&_descriptorSet,0,nullptr);

   vkCmdDispatch(_commandBufferCompute, (uint32_t)1024, 1, 1);

    if ( vkEndCommandBuffer ( _commandBufferCompute ) != VK_SUCCESS )
    {
      throw std::runtime_error ( "failed to record command buffer!" );
    }

    endSingleTimeCommands(_commandBufferCompute, _fence);

  
}

void vulkanApp::createSemaphores ( )
{
  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if ( vkCreateSemaphore ( _device,
                           &semaphoreInfo,
                           nullptr,
                           &_computeAvailableSemaphore ) != VK_SUCCESS)
  {

    throw std::runtime_error ( "failed to create semaphores!" );
  }
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
