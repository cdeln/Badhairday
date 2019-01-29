#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 varTexCoord[3];

out vec2 f_TexCoord;

uniform int outputLayer;

void main() {

    for(int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        f_TexCoord = varTexCoord[i];
        gl_Layer = outputLayer; 
        EmitVertex();
    }
    EndPrimitive();

}  
