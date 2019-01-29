
#include <iostream>
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "macros.h"

GLsizei windowWidth = 640;
GLsizei windowHeight = 480;

gtl::shader::Voxelizer voxelizer;

GLuint voxelGrid;
GLuint voxelLock;

Model * model;

Model * createModel(int numPoints) { 
    std::vector<glm::vec3> vertices;
    vertices.reserve(numPoints);
    for(int i = 0; i < numPoints; ++i) { 
        vertices.push_back(glm::vec3(0.5));
    }
    return LoadPointCloud( reinterpret_cast<GLfloat*>(&vertices[0]), vertices.size() );
}

void init(void) {

    gtl::debug::init();

    voxelizer.init();

    model = createModel(1); 

    std::vector<GLuint> lockInit(1, 0);
    // Init shaders

    // Init voxel grid
    /*
    glGenTextures(1, &voxelGrid);
    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, 1, 1, 1); 
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindImageTexture(0, voxelGrid, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
    */
    glGenTextures(1, &voxelGrid);
    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, 1, 1, 1); 
    //glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, 1, 1, 1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &lockInit[0]);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindImageTexture(0, voxelGrid, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glGenTextures(1, &voxelLock);
    glBindTexture(GL_TEXTURE_3D, voxelLock);
    //glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, numLayers, numLayers, numLayers);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, 1, 1, 1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &lockInit[0]);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindImageTexture(1, voxelLock, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glm::mat4 transform = glm::mat4();
    GLM_VERBOSE(transform); 

    voxelizer.use();
    voxelizer.transform = transform;
    DrawPointModel(model, voxelizer, "in_Position"); 
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glFinish();

    std::vector<GLuint> voxels(1,0);
    std::cout << "reading pixels" << std::endl;
    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &voxels[0]);
    std::cout << "pixels read" << std::endl;
    for(int i = 0; i < voxels.size(); ++i) {
        std::cout << voxels[i] << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitContextVersion(4, 5);
    glutInitWindowSize (windowWidth, windowHeight);
    glutCreateWindow ("Furball");
    glutRepeatingTimerFunc(20);
    //glutDisplayFunc(display);
    init();
    //glutMainLoop();
}
