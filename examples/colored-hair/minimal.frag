#version 150

in vec4 f_color;

out vec4 outColor;
//in vec3 f_exNormal; // Phong
//in vec3 f_exSurface; // Phong (specular)

void main(void)
{
	outColor = f_color; 
}
