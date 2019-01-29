#version 150

in vec2 varTexCoord;

out vec4 outColor;

uniform sampler2DArray sampler;
uniform int layer; 

void main(void)
{
    outColor = texture(sampler, vec3(varTexCoord, layer)); 
}
