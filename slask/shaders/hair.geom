#version 150

layout(triangles) in;
layout(line_strip, max_vertices = 2) out;

in vec3 exNormal[3];

out vec3 f_Normal;
out vec3 f_View;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    mat3 normalMatrix = mat3(modelViewMatrix);
	mat4 M = projectionMatrix * modelViewMatrix;

    vec3 mid_Position = vec3(0);
    vec3 mid_Normal = vec3(0);
    for(int i = 0; i < 3; ++i) {
        mid_Position += gl_in[i].gl_Position.xyz;
        mid_Normal += exNormal[i];
    }
    mid_Position /= 3;
    mid_Normal = normalize(mid_Normal);

    gl_Position = M * vec4(mid_Position,1);
    f_Normal = normalMatrix * mid_Normal;
    f_View = normalMatrix * mid_Position;
    EmitVertex();

    gl_Position = M * vec4(mid_Position + 0.1*mid_Normal, 1);
    f_Normal = normalMatrix * mid_Normal;
    f_View = normalMatrix * f_View;
    EmitVertex();

    EndPrimitive();
}  
