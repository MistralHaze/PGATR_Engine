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

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "./content/models/chalet.obj";
const std::string TEXTURE_PATH = "./content/textures/chalet.jpg";

const std::vector < const char* > validationLayers = {
  "VK_LAYER_KHRONOS_validation",
//  "VK_LAYER_LUNARG_standard_validation"
};

const std::vector < const char* > deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void vulkanApp::run ( )
{
  initWindow ( );
  initVulkan ( );
  mainLoop ( );
  cleanup ( );
}

void vulkanApp::initWindow ( )
{
  //Inicializacion glfw3
  glfwInit ( );

  //Config para no contexto ogl
  glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );

  //Creacion de la ventana (ultimos params, monitor y params para ogl)
  _window = glfwCreateWindow ( WIDTH, HEIGHT, "Vulkan", nullptr, nullptr );



  glfwSetWindowUserPointer ( _window, this );
  glfwSetWindowSizeCallback ( _window,
                              vulkanApp::onWindowResized );
}

void vulkanApp::initVulkan ( )
{
  //Init
  createInstance ( );
  setupDebugCallback ( );
  createSurface ( );
  pickPhysicalDevice ( );
  createLogicalDevice ( );

  //Renderconfig
  createSwapChain ( );
  createRenderPass ( );
  createGraphicsPipeline ( );

  //Commands
  createCommandPool ( );

  createDepthResources ( );
  createFramebuffers ( );

  createTextureImage ( );
  createTextureImageView ( );
  createTextureSampler ( );

  //Scene elements and synch!
  loadModel ( );

  createVertexBuffer ( );
  createIndexBuffer ( );
  createUniformBuffer ( );

  createDescriptorPool ( );
  createDescriptorSet ( );

  createCommandBuffers ( );
  createSemaphores ( );
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
  appInfo.pApplicationName = "Vulkan App";
  appInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo; //Aqui le pasamos el appInfo!!!

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

//3) Create the _surface. Es necesario para pasárselo al physical _device!!!
void vulkanApp::createSurface ( )
{
  if ( glfwCreateWindowSurface ( _instance, _window, nullptr, &_surface )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "Failed to create Window Surface!" );
  }
}

//4) Pillar el dispositivo físico
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
    if ( isDeviceSuitable ( ldevice ))
    {
      _physicalDevice = ldevice;
      break;
    }
  }

  if ( _physicalDevice == VK_NULL_HANDLE )
  {
    throw std::runtime_error ( "Failed to find a suitable device!" );
  }
}

//5)Create Logical _device
void vulkanApp::createLogicalDevice ( )
{
  QueueFamilyIndices lindices = findQueueFamilies ( _physicalDevice );

  std::vector < VkDeviceQueueCreateInfo > queueCreateInfos;
  std::set < int > uniqueQueueFamilies =
    { lindices._graphicsFamily, lindices._presentFamily };

  float queuePriority = 1.0f;
  for ( int queueFamily : uniqueQueueFamilies )
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back ( queueCreateInfo );
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.geometryShader = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount =
    static_cast<uint32_t>(queueCreateInfos.size ( ));
  createInfo.pQueueCreateInfos = queueCreateInfos.data ( );

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount =
    static_cast<uint32_t>(deviceExtensions.size ( ));
  createInfo.ppEnabledExtensionNames = deviceExtensions.data ( );

  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount =
      static_cast<uint32_t>(validationLayers.size ( ));
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

  vkGetDeviceQueue ( _device, lindices._graphicsFamily, 0, &_graphicsQueue );
  vkGetDeviceQueue ( _device, lindices._presentFamily, 0, &_presentQueue );
}

//6) Creación de la SwapChain!!!
void vulkanApp::createSwapChain ( )
{
  SwapChainSupportDetails
    swapChainSupport = querySwapChainSupport ( _physicalDevice );

  //Format, presenta y extend se hace desde los SwapChainSupportDetails!!
  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat ( swapChainSupport._formats );
  VkPresentModeKHR   presentMode = chooseSwapPresentMode ( swapChainSupport._presentModes );
  VkExtent2D         extent = chooseSwapExtent ( swapChainSupport._capabilities );

  uint32_t imageCount = swapChainSupport._capabilities.minImageCount + 1;
  if ( swapChainSupport._capabilities.maxImageCount > 0
    && imageCount > swapChainSupport._capabilities.maxImageCount )
  {
    imageCount = swapChainSupport._capabilities.maxImageCount;
  }

  //2-Creación del createInfo para la _swapChain.
  //Se rellena con la info de las funciones auxiliares
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = _surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1; //Siempre uno a no ser que sea una app
  // stereo o multitile
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //Este valor


  QueueFamilyIndices lindices = findQueueFamilies ( _physicalDevice );
  uint32_t queueFamilyIndices[] = { ( uint32_t ) lindices._graphicsFamily,
                                    ( uint32_t ) lindices._presentFamily };

  if ( lindices._graphicsFamily != lindices._presentFamily )
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; //La imagen
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //Accesible por
    // multiples queues a la vez. Requiere configuracion avanzada!!!
  }

  createInfo.preTransform =
    swapChainSupport._capabilities.currentTransform;

  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  //Modo presentacion (mail-box por defecto)
  createInfo.presentMode = presentMode;

  //Activar clipping
  createInfo.clipped = VK_TRUE;

  //Se crea la _swapChain al ginal
  if ( vkCreateSwapchainKHR ( _device, &createInfo, nullptr, &_swapChain )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create swap chain!" );
  }

  vkGetSwapchainImagesKHR ( _device, _swapChain, &imageCount, nullptr );
  _swapChainImages.resize ( imageCount );
  vkGetSwapchainImagesKHR ( _device,
                            _swapChain,
                            &imageCount,
                            _swapChainImages.data ( ));

  _swapChainImageFormat = surfaceFormat.format;
  _swapChainExtent = extent;

  //Here we create the image views for rendering
  createImageViews ( );

}

//7 Render pass
void vulkanApp::createRenderPass ( )
{
  //NOTA: Los parametros dont care suelen ser los maś rapidos, ya que no
  //llevan verificacion, pero por contra pueden dar problemas de
  //sincronizacion en la lectura/escritura en el framebuffer
  //Color
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = _swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  //Profundidad
  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format = findDepthFormat ( );
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  //
  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef = {};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1; //Esto es el layout del shader!!!
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  //###TODO: Testear los parametros de lso subpases mas en profundidad
  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;

  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;

  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
    | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  std::array < VkAttachmentDescription, 2 >
    attachments = { colorAttachment, depthAttachment };
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount =
    static_cast<uint32_t>(attachments.size ( ));
  renderPassInfo.pAttachments = attachments.data ( );
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (
    vkCreateRenderPass ( _device, &renderPassInfo, nullptr, &_renderPass )
      != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create render pass!" );
  }
}

//8 Creación de los descriptor Sets
void vulkanApp::createDescriptorSetLayout ( )
{
  VkDescriptorSetLayoutBinding uboLayoutBinding = {};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = nullptr;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array < VkDescriptorSetLayoutBinding, 2 > bindings = { uboLayoutBinding, samplerLayoutBinding };
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

//9) Creación del pipeline gráfico
void vulkanApp::createGraphicsPipeline ( )
{
  //Ya están los shaders compilados
  std::vector < char > vertShaderCode = readFile ( "./content/vk_shaders/geompassthrough/vert.spv" );
  std::vector < char > geomShaderCode = readFile ( "./content/vk_shaders/geompassthrough/geom.spv" );
  std::vector < char > fragShaderCode = readFile ( "./content/vk_shaders/geompassthrough/frag.spv" );

  VkShaderModule vertShaderModule = createShaderModule ( vertShaderCode );
  VkShaderModule geomShaderModule = createShaderModule ( geomShaderCode );
  VkShaderModule fragShaderModule = createShaderModule ( fragShaderCode );

  //Generacion y configuracion de cada una de las stages del pipeline
  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo geomShaderStageInfo = {};
  geomShaderStageInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
  geomShaderStageInfo.module = geomShaderModule;
  geomShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  //Generacion de las stages del pipeline
  VkPipelineShaderStageCreateInfo
    shaderStages[] = { vertShaderStageInfo, geomShaderStageInfo, fragShaderStageInfo };

  //Estas son todas las funciones fijas dentro del pipeline, que hay que
  //configurarlas de manera explicita.

  //Vertex Input: Espaceado de los vértices en los mallados a renderizar
  //------------------------------------------------------
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  //Bindings y attributes (hay que pasarlos como vectores de estructuras)
  auto bindingDescription = Vertex::getBindingDescription ( );
  auto attributeDescriptions = Vertex::getAttributeDescriptions ( );

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size ( ));
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data ( );
  //------------------------------------------------------
  //Input assembly: Definición de topología
  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  //El viewport (VkPipelineViewportStateCreateInfo) Se genera configurando el viewport y el scissor
  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = ( float ) _swapChainExtent.width;
  viewport.height = ( float ) _swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = { 0, 0 };
  scissor.extent = _swapChainExtent;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  //Raster
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  //Multisample
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType =
    VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  //Deph and stencil
  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;


  //Color blend (attachments and create info)
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
      | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType =
    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;


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

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 3;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = _pipelineLayout;
  pipelineInfo.renderPass = _renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if ( vkCreateGraphicsPipelines ( _device,
                                   VK_NULL_HANDLE,
                                   1,
                                   &pipelineInfo,
                                   nullptr,
                                   &_graphicsPipeline ) != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create graphics pipeline!" );
  }

  vkDestroyShaderModule ( _device, fragShaderModule, nullptr );
  vkDestroyShaderModule ( _device, geomShaderModule, nullptr );
  vkDestroyShaderModule ( _device, vertShaderModule, nullptr );
}


void vulkanApp::mainLoop ( )
{
  while ( !glfwWindowShouldClose ( _window ))
  {
    glfwPollEvents ( );

    updateUniformBuffer ( );
    drawFrame ( );
  }

  vkDeviceWaitIdle ( _device );
}

void vulkanApp::cleanupSwapChain ( )
{
  vkDestroyImageView ( _device, _depthImageView, nullptr );
  vkDestroyImage ( _device, _depthImage, nullptr );
  vkFreeMemory ( _device, _depthImageMemory, nullptr );

  for ( size_t i = 0; i < _swapChainFramebuffers.size ( ); i++ )
  {
    vkDestroyFramebuffer ( _device, _swapChainFramebuffers[i], nullptr );
  }

  vkFreeCommandBuffers ( _device,
                         _commandPool,
                         static_cast<uint32_t>(_commandBuffers.size ( )),
                         _commandBuffers.data ( ));

  vkDestroyPipeline ( _device, _graphicsPipeline, nullptr );
  vkDestroyPipelineLayout ( _device, _pipelineLayout, nullptr );
  vkDestroyRenderPass ( _device, _renderPass, nullptr );

  for ( size_t i = 0; i < _swapChainImageViews.size ( ); i++ )
  {
    vkDestroyImageView ( _device, _swapChainImageViews[i], nullptr );
  }

  vkDestroySwapchainKHR ( _device, _swapChain, nullptr );
}

void vulkanApp::cleanup ( )
{
  cleanupSwapChain ( );

  vkDestroySampler ( _device, _textureSampler, nullptr );
  vkDestroyImageView ( _device, _textureImageView, nullptr );

  vkDestroyImage ( _device, _textureImage, nullptr );
  vkFreeMemory ( _device, _textureImageMemory, nullptr );

  vkDestroyDescriptorPool ( _device, _descriptorPool, nullptr );

  vkDestroyDescriptorSetLayout ( _device, _descriptorSetLayout, nullptr );
  vkDestroyBuffer ( _device, _uniformBuffer, nullptr );
  vkFreeMemory ( _device, _uniformBufferMemory, nullptr );

  vkDestroyBuffer ( _device, _indexBuffer, nullptr );
  vkFreeMemory ( _device, _indexBufferMemory, nullptr );

  vkDestroyBuffer ( _device, _vertexBuffer, nullptr );
  vkFreeMemory ( _device, _vertexBufferMemory, nullptr );

  vkDestroySemaphore ( _device, _renderFinishedSemaphore, nullptr );
  vkDestroySemaphore ( _device, _imageAvailableSemaphore, nullptr );

  vkDestroyCommandPool ( _device, _commandPool, nullptr );

  vkDestroyDevice ( _device, nullptr );
  DestroyDebugReportCallbackEXT ( _instance, _callback, nullptr );
  vkDestroySurfaceKHR ( _instance, _surface, nullptr );


  //Destruir la intancia al final de todo! -> Ultimo objeto de vullkan a
  //destruir
  vkDestroyInstance ( _instance, nullptr );

  //Destroy glfw _window and events
  glfwDestroyWindow ( _window );
  glfwTerminate ( );
}

void vulkanApp::onWindowResized ( GLFWwindow* window,
                                   int width,
                                   int height )

{
  if ( width == 0 || height == 0 )
    return;

  vulkanApp* app =
    reinterpret_cast<vulkanApp*>(glfwGetWindowUserPointer (
      window ));
  app->recreateSwapChain ( );
}

//Recrear la swapchain, por ejemplo, al resizear la ventana
void vulkanApp::recreateSwapChain ( )
{
  //Terminar lo que este haciengo
  vkDeviceWaitIdle ( _device );

  //
  cleanupSwapChain ( );

  //Los creates iniciales. Recrear todo la swap chain y todo lo que depende de ella
  //Incluido lo que depende de la ventana
  createSwapChain ( );
  createRenderPass ( );
  createGraphicsPipeline ( );
  createDepthResources ( );
  createFramebuffers ( );
  createCommandBuffers ( );
}


void vulkanApp::createImageViews ( )
{
  _swapChainImageViews.resize ( _swapChainImages.size ( ));

  for ( uint32_t i = 0; i < _swapChainImages.size ( ); i++ )
  {
    //Cada image _view requiere su propia configuración
    _swapChainImageViews[i] = createImageView ( _swapChainImages[i],
                                               _swapChainImageFormat,
                                               VK_IMAGE_ASPECT_COLOR_BIT );
  }
}


void vulkanApp::createFramebuffers ( )
{
  _swapChainFramebuffers.resize ( _swapChainImageViews.size ( ));

  for ( size_t i = 0; i < _swapChainImageViews.size ( ); i++ )
  {
    std::array < VkImageView, 2 > attachments = {
      _swapChainImageViews[i],
      _depthImageView
    };

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount =
      static_cast<uint32_t>(attachments.size ( ));
    framebufferInfo.pAttachments = attachments.data ( );
    framebufferInfo.width = _swapChainExtent.width;
    framebufferInfo.height = _swapChainExtent.height;
    framebufferInfo.layers = 1;

    if ( vkCreateFramebuffer ( _device,
                               &framebufferInfo,
                               nullptr,
                               &_swapChainFramebuffers[i] ) != VK_SUCCESS )
    {
      throw std::runtime_error ( "failed to create framebuffer!" );
    }
  }
}

void vulkanApp::createCommandPool ( )
{
  //Encontrar los _indices de las colas (gráficas para este ejemplo)
  QueueFamilyIndices
    queueFamilyIndices = findQueueFamilies ( _physicalDevice );

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices._graphicsFamily;

  if ( vkCreateCommandPool ( _device, &poolInfo, nullptr, &_commandPool )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create graphics command pool!" );
  }
}

void vulkanApp::createDepthResources ( )
{
  VkFormat depthFormat = findDepthFormat ( );

  createImage ( _swapChainExtent.width,
                _swapChainExtent.height,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _depthImage,
                _depthImageMemory );
  _depthImageView =
    createImageView ( _depthImage,
                      depthFormat,
                      VK_IMAGE_ASPECT_DEPTH_BIT );

  transitionImageLayout ( _depthImage,
                          depthFormat,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
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

VkFormat vulkanApp::findDepthFormat ( )
{
  return findSupportedFormat (
    { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT },
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

bool vulkanApp::hasStencilComponent ( VkFormat format )
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT
    || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void vulkanApp::createTextureImage ( )
{
  //###
  unsigned int texWidth, texHeight;//, texChannels;
  unsigned int texChannels;
  unsigned char* pixels = loadTexture ( TEXTURE_PATH.c_str ( )
                                        ,texWidth
                                        ,texHeight
                                        ,texChannels
                                        );

  VkDeviceSize imageSize = texWidth*texHeight*texChannels;

  if ( !pixels )
  {
    throw std::runtime_error ( "failed to load texture image!" );
  }

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer ( imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory );

  void* data;
  vkMapMemory ( _device, stagingBufferMemory, 0, imageSize, 0, &data );
  memcpy ( data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory ( _device, stagingBufferMemory );

  createImage ( texWidth,
                texHeight,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT
                  | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                _textureImage,
                _textureImageMemory );

  transitionImageLayout ( _textureImage,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
  copyBufferToImage ( stagingBuffer,
                      _textureImage,
                      static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
  transitionImageLayout ( _textureImage,
                          VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  vkDestroyBuffer ( _device, stagingBuffer, nullptr );
  vkFreeMemory ( _device, stagingBufferMemory, nullptr );
}

void vulkanApp::createTextureImageView ( )
{
  _textureImageView = createImageView ( _textureImage,
                                       VK_FORMAT_R8G8B8A8_UNORM,
                                       VK_IMAGE_ASPECT_COLOR_BIT );
}

void vulkanApp::createTextureSampler ( )
{
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if ( vkCreateSampler ( _device, &samplerInfo, nullptr, &_textureSampler )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create texture sampler!" );
  }
}

//Creación de image views (para el swapchain, profundidad, texturas etc)
VkImageView vulkanApp::createImageView ( VkImage image,
                                         VkFormat format,
                                         VkImageAspectFlags aspectFlags )
{
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if ( vkCreateImageView ( _device, &viewInfo, nullptr, &imageView )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create texture image _view!" );
  }

  return imageView;
}

void vulkanApp::createImage ( uint32_t width,
                              uint32_t height,
                              VkFormat format,
                              VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkImage& image,
                              VkDeviceMemory& imageMemory )
{
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if ( vkCreateImage ( _device, &imageInfo, nullptr, &image )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to create image!" );
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements ( _device, image, &memRequirements );

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
    findMemoryType ( memRequirements.memoryTypeBits, properties );

  if ( vkAllocateMemory ( _device, &allocInfo, nullptr, &imageMemory )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to allocate image memory!" );
  }

  vkBindImageMemory ( _device, image, imageMemory, 0 );
}

void vulkanApp::transitionImageLayout ( VkImage image,
                                        VkFormat format,
                                        VkImageLayout oldLayout,
                                        VkImageLayout newLayout )
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands ( );

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;

  if ( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if ( hasStencilComponent ( format ))
    {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }
  else
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
    && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
    && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
      | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else
  {
    throw std::invalid_argument ( "unsupported layout transition!" );
  }

  vkCmdPipelineBarrier (
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  endSingleTimeCommands ( commandBuffer );
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

void vulkanApp::loadModel ( )
{
  Assimp::Importer Importer;
  const aiScene* pScene = Importer.ReadFile(MODEL_PATH.c_str ( ), aiProcess_Triangulate | aiProcess_CalcTangentSpace);

  const aiMesh* paiMesh = pScene->mMeshes[0];

  std::vector<Vertex> Vertices;
  std::vector<unsigned int> Indices;
  const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

  std::unordered_map < Vertex, uint32_t > uniqueVertices = {};

  for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++)
  {
    const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
    //const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
    const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

    Vertex vertex = {};

    vertex._pos = {pPos->x, pPos->y, pPos->z};
    vertex._texCoord = {pTexCoord->x, pTexCoord->y};
    vertex._color = { 1.0f, 1.0f, 1.0f };

    if ( uniqueVertices.count ( vertex ) == 0 )
    {
      uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size ( ));
      _vertices.push_back ( vertex );
    }
    _indices.push_back ( uniqueVertices[vertex] );
  }
}


void vulkanApp::createVertexBuffer ( )
{
  VkDeviceSize bufferSize = sizeof ( _vertices[0] )*_vertices.size ( );

  //Creación de los staging buffers
  //Visible desde CPU
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory );

  //Se copian los datos
  void* data;
  vkMapMemory ( _device, stagingBufferMemory, 0, bufferSize, 0, &data );
  memcpy ( data, _vertices.data ( ), ( size_t ) bufferSize );
  vkUnmapMemory ( _device, stagingBufferMemory );

  //Sólo visible desde la GPU
  createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT
                   | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 _vertexBuffer,
                 _vertexBufferMemory );

  //Se copia de un buffer a otro
  copyBuffer ( stagingBuffer, _vertexBuffer, bufferSize );

  //Se destruye el temporal usado para la CPU transfer
  vkDestroyBuffer ( _device, stagingBuffer, nullptr );
  vkFreeMemory ( _device, stagingBufferMemory, nullptr );
}

void vulkanApp::createIndexBuffer ( )
{
  VkDeviceSize bufferSize = sizeof ( _indices[0] )*_indices.size ( );

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer,
                 stagingBufferMemory );

  void* data;
  vkMapMemory ( _device, stagingBufferMemory, 0, bufferSize, 0, &data );
  memcpy ( data, _indices.data ( ), ( size_t ) bufferSize );
  vkUnmapMemory ( _device, stagingBufferMemory );

  createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT
                   | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 _indexBuffer,
                 _indexBufferMemory );

  copyBuffer ( stagingBuffer, _indexBuffer, bufferSize );

  vkDestroyBuffer ( _device, stagingBuffer, nullptr );
  vkFreeMemory ( _device, stagingBufferMemory, nullptr );
}

void vulkanApp::createUniformBuffer ( )
{
  VkDeviceSize bufferSize = sizeof ( UniformBufferObject );
  createBuffer ( bufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 _uniformBuffer,
                 _uniformBufferMemory );
}

void vulkanApp::createDescriptorPool ( )
{
  std::array < VkDescriptorPoolSize, 2 > poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 1;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 1;

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
  bufferInfo.buffer = _uniformBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = sizeof ( UniformBufferObject );

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = _textureImageView;
  imageInfo.sampler = _textureSampler;

  std::array < VkWriteDescriptorSet, 2 > descriptorWrites = {};

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = _descriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pBufferInfo = &bufferInfo;

  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = _descriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].dstArrayElement = 0;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].pImageInfo = &imageInfo;

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

  vkQueueSubmit ( _graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
  vkQueueWaitIdle ( _graphicsQueue );

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
  _commandBuffers.resize ( _swapChainFramebuffers.size ( ));

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

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _swapChainExtent;

    //Color de limpiado
    std::array < VkClearValue, 2 > clearValues = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };

    //Efecto tormenta, el _color de limpiado es diferente en cada frambuffer se aprecia que no es regular el render
    //if (i%2==0) clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    //else        clearValues[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };

    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount =
      static_cast<uint32_t>(clearValues.size ( ));
    renderPassInfo.pClearValues = clearValues.data ( );

    vkCmdBeginRenderPass ( _commandBuffers[i],
                           &renderPassInfo,
                           VK_SUBPASS_CONTENTS_INLINE );

    vkCmdBindPipeline ( _commandBuffers[i],
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        _graphicsPipeline );

    VkBuffer vertexBuffers[] = { _vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers ( _commandBuffers[i],
                             0,
                             1,
                             vertexBuffers,
                             offsets );

    vkCmdBindIndexBuffer ( _commandBuffers[i],
                           _indexBuffer,
                           0,
                           VK_INDEX_TYPE_UINT32 );

    vkCmdBindDescriptorSets ( _commandBuffers[i],
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              _pipelineLayout,
                              0,
                              1,
                              &_descriptorSet,
                              0,
                              nullptr );

    vkCmdDrawIndexed ( _commandBuffers[i],
                       static_cast<uint32_t>(_indices.size ( )),
                       1,
                       0,
                       0,
                       0 );

    vkCmdEndRenderPass ( _commandBuffers[i] );

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
  ubo._proj = glm::perspective ( glm::radians ( 45.0f ),
                                _swapChainExtent.width
                                  /( float ) _swapChainExtent.height,
                                0.1f,
                                10.0f );
  ubo._proj[1][1] *= -1; // switching from OGL to Vulkan sys. coord.

  void* data;
  vkMapMemory ( _device,
                _uniformBufferMemory,
                0,
                sizeof ( ubo ),
                0,
                &data );
  memcpy ( data, &ubo, sizeof ( ubo ));
  vkUnmapMemory ( _device, _uniformBufferMemory );
}

void vulkanApp::drawFrame ( )
{
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR ( _device,
                                            _swapChain,
                                            std::numeric_limits < uint64_t >::max ( ),
                                            _imageAvailableSemaphore,
                                            VK_NULL_HANDLE,
                                            &imageIndex );

  if ( result == VK_ERROR_OUT_OF_DATE_KHR )
  {
    recreateSwapChain ( );
    return;
  }
  else if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
  {
    throw std::runtime_error ( "failed to acquire swap chain image!" );
  }

  VkSemaphore waitSemaphores[]      = { _imageAvailableSemaphore };
  VkSemaphore signalSemaphores[]    = { _renderFinishedSemaphore };
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];


  if ( vkQueueSubmit ( _graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE )
    != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to submit draw command buffer!" );
  }

  //Aquí es donde se envía para su presentacion
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = { _swapChain };
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR ( _presentQueue, &presentInfo );

  if ( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR )
  {
    recreateSwapChain ( );
  }
  else if ( result != VK_SUCCESS )
  {
    throw std::runtime_error ( "failed to present swap chain image!" );
  }

  vkQueueWaitIdle ( _presentQueue );
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

VkSurfaceFormatKHR vulkanApp::chooseSwapSurfaceFormat ( const std::vector <
  VkSurfaceFormatKHR >& availableFormats )
{
  if ( availableFormats.size ( ) == 1
    && availableFormats[0].format == VK_FORMAT_UNDEFINED )
  {
    return { VK_FORMAT_B8G8R8A8_UNORM,
             VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  }

  for ( const auto& availableFormat : availableFormats )
  {
    if ( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
      && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
    {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR vulkanApp::chooseSwapPresentMode ( const std::vector <
  VkPresentModeKHR > availablePresentModes )
{
  VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

  for ( const auto& availablePresentMode : availablePresentModes )
  {
    if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
    {
      return availablePresentMode;
    }
    else if ( availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
    {
      bestMode = availablePresentMode;
    }
  }

  return bestMode;
}


//Escoger la mejor resolucion acorde al tamanyo de la ventana
VkExtent2D vulkanApp::chooseSwapExtent ( const VkSurfaceCapabilitiesKHR& capabilities )
{
  if ( capabilities.currentExtent.width
    != std::numeric_limits < uint32_t >::max ( ))
  {
    return capabilities.currentExtent;
  }
  else
  {
    int width, height;
    glfwGetWindowSize ( _window, &width, &height );

    VkExtent2D actualExtent = {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
    };

    actualExtent.width = std::max ( capabilities.minImageExtent.width,
                                    std::min ( capabilities.maxImageExtent
                                                           .width,
                                               actualExtent.width ));
    actualExtent.height = std::max ( capabilities.minImageExtent.height,
                                     std::min ( capabilities
                                                  .maxImageExtent
                                                  .height,
                                                actualExtent.height ));

    return actualExtent;
  }
}

//Devuelve todo el soporte para swapchain que tiene el dispositivo dado
SwapChainSupportDetails vulkanApp::querySwapChainSupport ( VkPhysicalDevice ldevice )
{
  //Tipo específico para almacenar info del soporte de la SwapChain
  SwapChainSupportDetails details;

  //Surface _capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( ldevice,
                                              _surface,
                                              &details._capabilities );

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR ( ldevice,
                                         _surface,
                                         &formatCount,
                                         nullptr );
  //Format
  if ( formatCount != 0 )
  {
    details._formats.resize ( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR ( ldevice,
                                           _surface,
                                           &formatCount,
                                           details._formats.data ( ));
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR ( ldevice,
                                              _surface,
                                              &presentModeCount,
                                              nullptr );
  //Presentation modes
  if ( presentModeCount != 0 )
  {
    details._presentModes.resize ( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR ( ldevice,
                                                _surface,
                                                &presentModeCount,
                                                details._presentModes
                                                       .data ( ));
  }

  return details;
}


bool vulkanApp::isDeviceSuitable ( VkPhysicalDevice ldevice )
{
  QueueFamilyIndices lindices = findQueueFamilies ( ldevice );

  bool extensionsSupported = checkDeviceExtensionSupport ( ldevice );

  if ( extensionsSupported )
  {
    SwapChainSupportDetails
      swapChainSupport = querySwapChainSupport ( ldevice );
    //swapChainAdequate = !swapChainSupport._formats.empty() &&
    //  !swapChainSupport._presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures ( ldevice, &supportedFeatures );

  return  lindices.isComplete ( )
          && extensionsSupported
          && supportedFeatures.samplerAnisotropy
          && supportedFeatures.geometryShader
          ;
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
  QueueFamilyIndices lindices;

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
    //Es una cola grafica?
    if ( queueFamily.queueCount > 0
      && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
    {
      lindices._graphicsFamily = i;
    }

    //Para determinar si esta cola posee capacidad de presentacion
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR ( ldevice,
                                           i,
                                           _surface,
                                           &presentSupport );

    //Posee capacidades de presentacion?
    if ( queueFamily.queueCount > 0 && presentSupport )
    {
      lindices._presentFamily = i;
    }

    if ( lindices.isComplete ( ))
    {
      break;
    }

    i++;
  }

  return lindices;
}


std::vector < const char* > vulkanApp::getRequiredExtensions ( )
{
  std::vector < const char* > extensions;

  unsigned int glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions =
    glfwGetRequiredInstanceExtensions ( &glfwExtensionCount );

  for ( unsigned int i = 0; i < glfwExtensionCount; i++ )
  {
    extensions.push_back ( glfwExtensions[i] );
  }

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

  ( void ) flags;
  ( void ) objType;
  ( void ) obj;
  ( void ) location;
  ( void ) code;
  ( void ) layerPrefix;
  ( void ) userData;

  return VK_FALSE;
}
