#version 150

in  vec3 inPosition;
in  vec3 inNormal;

out vec3 f_Normal;
out vec3 f_View;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main(void)
{
    mat3 normalMatrix = mat3(modelViewMatrix);
    mat4 M = projectionMatrix * modelViewMatrix;
    f_Normal = normalMatrix * inNormal; 
    gl_Position = M * vec4(inPosition, 1.0); 
    f_View = vec3(modelViewMatrix * vec4(inPosition, 1.0)); //gl_Position.xyz / gl_Position.w; 
}
