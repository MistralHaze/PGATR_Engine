#version 450
#extension GL_ARB_separate_shader_objects : enable

/* geometry shader passthrough */
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

const int numvert = 3;

/* input */
layout(location = 0) in vec3 geomColor[numvert];
layout(location = 1) in vec2 geomTexCoord[numvert];

/* output */
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
	for (int i = 0; i < numvert; i++)
	{
		gl_Position = gl_in[i].gl_Position;
		fragColor = geomColor[i];
		fragTexCoord = geomTexCoord[i];
		EmitVertex();
	}
    EndPrimitive();
}
