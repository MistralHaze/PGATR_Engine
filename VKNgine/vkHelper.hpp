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

#include <FreeImage.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void loadMesh ( const std::string& file
                ,float* vertexPos
                ,float* vertexNormal
                ,float* vertexTexCoord
                ,float* vertexTangent
                ,unsigned int* triangleIndex
                )
{

  Assimp::Importer importer;
  const aiScene* scene = nullptr;
  aiMesh* mesh = nullptr;

  scene = importer
    .ReadFile ( file, aiProcess_Triangulate | aiProcess_CalcTangentSpace );

  if ( !scene )
  {
    std::cout << "file " + file + " no found" << std::endl;
    exit ( 1 );
  }

  mesh = scene->mMeshes[0];
  if ( !mesh )
  {
    std::cout << "mesh not found in " + file << std::endl;
    exit ( 1 );
  }

  unsigned int numVertices = mesh->mNumVertices;
  unsigned int numTriangles = mesh->mNumFaces;

  vertexPos       = new float[3*numVertices];
  vertexNormal    = new float[3*numVertices];
  vertexTexCoord  = new float[2*numVertices];
  vertexTangent   = new float[3*numVertices];
  triangleIndex   = new unsigned[3*numTriangles];

  for ( unsigned i = 0; i < numVertices; ++i )
  {
    aiVector3D ver = mesh->mVertices[i];
    vertexPos[3*i] = ver.x;
    vertexPos[3*i + 1] = ver.y;
    vertexPos[3*i + 2] = ver.z;
  }

  for ( unsigned i = 0; i < numVertices; ++i )
  {
    aiVector3D nor = mesh->mNormals[i];
    vertexNormal[3*i] = nor.x;
    vertexNormal[3*i + 1] = nor.y;
    vertexNormal[3*i + 2] = nor.z;
  }

  for ( unsigned i = 0; i < numVertices; ++i )
  {
    aiVector3D tex = mesh->mTextureCoords[0][i];
    vertexTexCoord[2*i] = tex.x;
    vertexTexCoord[2*i + 1] = tex.y;
  }

  for ( unsigned i = 0; i < numVertices; ++i )
  {
    aiVector3D tan = mesh->mTangents[i];
    vertexTangent[3*i] = tan.x;
    vertexTangent[3*i + 1] = tan.y;
    vertexTangent[3*i + 2] = tan.z;
  }

  for ( unsigned i = 0; i < numTriangles; ++i )
  {
    unsigned* tri = mesh->mFaces[i].mIndices;
    triangleIndex[3*i] = tri[0];
    triangleIndex[3*i + 1] = tri[1];
    triangleIndex[3*i + 2] = tri[2];
  }
}


unsigned char* loadTexture ( const char* fileName,
                            unsigned int& w,
                            unsigned int& h,
                            unsigned int& c
                            )
{
  FreeImage_Initialise ( TRUE );

  FREE_IMAGE_FORMAT format = FreeImage_GetFileType ( fileName, 0 );
  if ( format == FIF_UNKNOWN )
    format = FreeImage_GetFIFFromFilename ( fileName );
  if (( format == FIF_UNKNOWN ) || !FreeImage_FIFSupportsReading ( format ))
    return NULL;

  FIBITMAP* img = FreeImage_Load ( format, fileName );
  if ( img == NULL )
    return NULL;

  FIBITMAP* tempImg = img;
  img = FreeImage_ConvertTo32Bits ( img );
  FreeImage_Unload ( tempImg );

  w = FreeImage_GetWidth ( img );
  h = FreeImage_GetHeight ( img );
  c = 4;

  //BGRA toVkVertexInputBindingDescription RGBA
  unsigned char* map = new unsigned char[c*w*h];
  char* buff = ( char* ) FreeImage_GetBits ( img );

  for ( unsigned int j = 0; j < w*h; j++ )
  {
    map[j*4 + 0] = buff[j*4 + 2];
    map[j*4 + 1] = buff[j*4 + 1];
    map[j*4 + 2] = buff[j*4 + 0];
    map[j*4 + 3] = buff[j*4 + 3];
  }

  FreeImage_Unload ( img );
  FreeImage_DeInitialise ( );

  return map;
}
