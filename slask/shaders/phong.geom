#version 150

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 exNormal[3];

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	//f_exNormal = exNormal[0];
	//f_exSurface = exSurface[0];
	mat4 M = projectionMatrix * modelviewMatrix;

	for(int i = 0; i < 3; ++i) {
		gl_Position = M * gl_in[i].gl_Position; //+ vec4(-0.1, 0.0, 0.0, 0.0);
		EmitVertex();

		gl_Position = M * (gl_in[i].gl_Position + 1000*vec4(exNormal[i], 0));
		EmitVertex();

		EndPrimitive();
	}
}  
