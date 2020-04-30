#version 450

const int numvert = 4;

layout(triangles) in;
layout(line_strip, max_vertices = numvert) out;

//avoid z fighting by pushing the wireframe nearer
const vec4 tras = vec4(0.0, 0.0, -0.1, 0.0);

void main()
{
	for (int i = 0; i < numvert; i++)
	{
		gl_Position = tras + gl_in[i].gl_Position;
		EmitVertex();
	}
    EndPrimitive();
}
