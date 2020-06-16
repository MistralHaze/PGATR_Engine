#version 450 core
 
layout(triangles, equal_spacing, ccw) in;

const int numvert = 3;

layout(location = 0) in vec3 tcColor[gl_MaxPatchVertices];
layout(location = 1) in vec3 tcPos[gl_MaxPatchVertices];
layout(location = 2) in vec2 tcTexCoord[gl_MaxPatchVertices];

layout(location = 0) out vec3 teColor;
layout(location = 1) out vec3 tePos;
layout(location = 2) out vec2 teTexCoord;

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2);
vec3 interpolate(in vec3 v0, in vec3 v1, in vec3 v2);
vec2 interpolate(in vec2 v0, in vec2 v1, in vec2 v2);

void main()
{ 
	teColor = interpolate(
	tcColor[0], 
	tcColor[1],  
	tcColor[2]);

	tePos = interpolate(
	tcPos[0], 
	tcPos[1],  
	tcPos[2]);

	teTexCoord = interpolate(
	tcTexCoord[0], 
	tcTexCoord[1],  
	tcTexCoord[2]);

	vec4 pos = interpolate(
	gl_in[0].gl_Position, 
	gl_in[1].gl_Position, 
	gl_in[2].gl_Position);


	//sin desplazamiento
	gl_Position = pos;
	
}

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2)
{
	vec4 l1 = v1 - v0;
	vec4 l0 = v2 - v0;

	return v0 + gl_TessCoord.x * l1 + gl_TessCoord.y * l0;
}


vec3 interpolate(in vec3 v0, in vec3 v1, in vec3 v2)
{
	vec3 l1 = v1 - v0;
	vec3 l0 = v2 - v0;

	return v0 + gl_TessCoord.x * l1 + gl_TessCoord.y * l0;
}


vec2 interpolate(in vec2 v0, in vec2 v1, in vec2 v2)
{
	vec2 l1 = v1 - v0;
	vec2 l0 = v2 - v0;

	return v0 + gl_TessCoord.x * l1 + gl_TessCoord.y * l0;
}