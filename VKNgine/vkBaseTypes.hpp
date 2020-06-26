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

//Queues Families
struct QueueFamilyIndices
{

  int _computeFamily = -1;

  bool isComplete ( )
  {
    return _computeFamily >= 0;
  }
};

//Estructura con los 3 parametros que se manejan para configurar una swapchain
struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR _capabilities;
  std::vector < VkSurfaceFormatKHR > _formats;
  std::vector < VkPresentModeKHR > _presentModes;
};

//Definición de vértice (pos, col and tex).
struct Vertex
{
  glm::vec3 _pos;
  glm::vec3 _color;
  glm::vec2 _texCoord;

  static VkVertexInputBindingDescription getBindingDescription ( )
  {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof ( Vertex );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array < VkVertexInputAttributeDescription,
    3 > getAttributeDescriptions ( )
  {
    std::array < VkVertexInputAttributeDescription, 3 >
      attributeDescriptions = {};

    //Posicion
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof( Vertex, _pos );

    //Color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof( Vertex, _color );

    //TexCoords
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof( Vertex, _texCoord );

    return attributeDescriptions;
  }

  bool operator== ( const Vertex& other ) const
  {
    return _pos == other._pos
           && _color == other._color
           && _texCoord == other._texCoord;
  }
};

namespace std
{
  template <> struct hash < Vertex >
  {
    size_t operator() ( Vertex const& vertex ) const
    {
      return (( hash < glm::vec3 > ( ) ( vertex._pos )
              ^ ( hash < glm::vec3 > ( ) ( vertex._color ) << 1 )) >> 1 )
              ^ ( hash < glm::vec2 > ( ) ( vertex._texCoord ) << 1 );
    }
  };
}

struct UniformBufferObject
{
  glm::mat4 _model;
  glm::mat4 _view;
  glm::mat4 _proj;
};
