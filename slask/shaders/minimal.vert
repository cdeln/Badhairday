#version 150

in  vec3 inPosition;
in  vec3 inNormal;

out vec3 exNormal;

void main(void)
{
	exNormal = inNormal; 
	gl_Position = vec4(inPosition, 1.0); 
}
