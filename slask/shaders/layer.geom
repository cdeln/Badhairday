#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 modelToScreen;
uniform int numLayers;
uniform int curLayer;

void main() {

    for(int i = 0; i < 3; ++i) {
        gl_Position = modelToScreen * gl_in[i].gl_Position;
        gl_Position /= gl_Position.w;
        gl_Layer = int(numLayers * (gl_Position.z + 1) / 2);
        EmitVertex();
    }
    EndPrimitive();

}  
