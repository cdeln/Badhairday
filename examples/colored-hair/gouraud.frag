#version 150

out vec4 outColor;
in float shade;

void main(void)
{
	outColor = vec4(shade, shade, shade, 1.0);
}
