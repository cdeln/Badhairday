
#include <iostream>
#include "common_ext.hpp"

int windowWidth = 100;
int windowHeight = 100;

void init(void) {

    // dimensions of the image
    int tex_w = 512, tex_h = 512;
    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    const char * the_ray_shader_string = 
        "	#version 430"
        "		layout(local_size_x = 1, local_size_y = 1) in;"
        "	layout(rgba32f, binding = 0) uniform image2D img_output;"
        "	void main() {"
        "		vec4 pixel = vec4(0.0, 0.0, 0.0, 1.0);"
        "		ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);"
        "		imageStore(img_output, pixel_coords, pixel);"
        "	}"
        ;

    GLuint ray_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(ray_shader, 1, &the_ray_shader_string, NULL);
    glCompileShader(ray_shader);
    // check for compilation errors as per normal here

    GLuint ray_program = glCreateProgram();
    glAttachShader(ray_program, ray_shader);
    glLinkProgram(ray_program);
    // check for linking errors and validate program as per normal here

}

void display() {

    glutSwapBuffers();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA); 
    glutInitContextVersion(4, 5);
    glutInitWindowSize (windowWidth, windowHeight);
    glutCreateWindow ("Furball");
    glutRepeatingTimerFunc(20);
    glutDisplayFunc(display);
    init();
    glutMainLoop();
}
