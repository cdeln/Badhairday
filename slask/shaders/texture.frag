#version 150

in vec2 varTexCoord;

out vec4 outColor;

uniform sampler2D sampler;

void main(void)
{
    outColor = texture(sampler, varTexCoord);
}
