#version 150

in vec2 varTexCoord;

out vec4 outColor;

uniform sampler3D sampler;
uniform float z_TexCoord;

void main(void)
{
    outColor = texture(sampler, vec3(varTexCoord, z_TexCoord)); 
}
