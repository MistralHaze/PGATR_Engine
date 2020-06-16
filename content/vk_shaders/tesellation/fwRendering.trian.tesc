#version 450 core
 
layout(vertices = 3) out;

const int numvert = 3;

layout(location = 0) in vec3 vPos[gl_MaxPatchVertices];
layout(location = 1) in vec3 vColor[gl_MaxPatchVertices];
layout(location = 2) in vec2 vTexCoord[gl_MaxPatchVertices];

layout(location = 0) out vec3 tcColor[numvert];
layout(location = 1) out vec3 tcPos[numvert];
layout(location = 2) out vec2 tcTexCoord[numvert];

const int outerLevel=2;
const int innerLevel=2;
 
void main(void)
{
	 gl_TessLevelOuter[0] = outerLevel;
	 gl_TessLevelOuter[1] = outerLevel;
	 gl_TessLevelOuter[2] = outerLevel;
 
	 gl_TessLevelInner[0] = innerLevel;
	 
	tcColor[gl_InvocationID] = vColor[gl_InvocationID];

	tcPos[gl_InvocationID] = vPos[gl_InvocationID];

	tcTexCoord[gl_InvocationID] = vTexCoord[gl_InvocationID];
 
	 gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
