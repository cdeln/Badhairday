#version 150

in vec2 f_TexCoord;

out vec4 outColor;

uniform sampler2DArray sampler;
uniform int inputLayer;

void main(void)
{
    outColor = texture(sampler, vec3(f_TexCoord, inputLayer));
}
