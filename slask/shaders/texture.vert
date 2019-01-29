#version 150

in vec3 inPosition;
in vec2 inTexCoord;

out vec2 varTexCoord;

void main(void)
{
    varTexCoord = inTexCoord;
    gl_Position = vec4(inPosition, 1.0);
}
