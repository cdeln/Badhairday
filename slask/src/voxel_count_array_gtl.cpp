
#include <iostream>
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "framebuffer.hpp"

glm::mat4 viewFromWorld; 
glm::mat4 projectionMatrix;
glm::vec3 lightOrg;

gtl::shader::Texture2DArray textureShader;
gtl::shader::VoxelCount voxelCountShader;
gtl::shader::AddLayers cumsumShader;

GLfloat curTime;
GLfloat angle;

GLsizei windowWidth = 640;
GLsizei windowHeight = 480;

gtl::Framebuffer<GL_TEXTURE_2D_ARRAY> voxelFBO;

Model * quad;
Model * ball;

int curLayer = 0;
int numLayers = 10;

void setUniforms() {
    glViewport(0, 0, windowWidth, windowHeight); 
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    projectionMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 1.25f, 2.75f);
}

void keyboardCallback(unsigned char event, int x, int y) {
    curLayer = static_cast<int>(event) - 48;
    std::cout << "curLayer = " << curLayer << std::endl;
}

void init(void) {

    textureShader.init();
    voxelCountShader.init();
    cumsumShader.init();

    voxelFBO.init();
    voxelFBO.allocate(windowWidth, windowHeight, numLayers);

    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glLineWidth(1);

    quad = util::quad();
    ball = util::ball(0.5);

    viewFromWorld = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    lightOrg = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

    setUniforms();

    textureShader.use();
}

void display(void)
{
    voxelFBO.bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curTime = glutGet(GLUT_ELAPSED_TIME);	
    angle = 0.001f * curTime; 

    glm::mat4 worldFromModel = glm::rotate(angle, glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 modelViewMatrix = viewFromWorld * worldFromModel;

    glm::mat3 lightRotation = glm::mat3(glm::rotate(2.3f*angle, glm::vec3(0.0f,1.0f,0.0f)));
    glm::vec3 light = lightRotation * lightOrg; 

    glm::mat4 modelToScreen = projectionMatrix * modelViewMatrix; 
    voxelCountShader.use();
    voxelCountShader.modelToScreen = &modelToScreen[0][0];
    voxelCountShader.numLayers = numLayers;
    voxelCountShader.curLayer = curLayer; 
    DrawModel(ball, voxelCountShader, "inPosition", NULL, NULL); 

    // Cumsum
    voxelFBO.bind();

    glBlendFunc(GL_ONE, GL_ONE);

    voxelFBO.texture.bind(0);
    cumsumShader.use();
    cumsumShader.sampler = voxelFBO.texture.unit();
    for(int i = 0; i < numLayers - 1; ++i) {
        cumsumShader.inputLayer = i + 0;
        cumsumShader.outputLayer = i + 1;
        DrawModel(quad, cumsumShader, "inPosition", NULL, "inTexCoord");
    }


    // Draw onto screen
    voxelFBO.unbind();
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    voxelFBO.texture.bind(0);
    textureShader.use();
    textureShader.sampler = voxelFBO.texture.unit(); 
    textureShader.layer = curLayer;
    DrawModel(quad, textureShader, "inPosition", NULL, "inTexCoord");

    printError("display");

    glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h) {
    windowWidth = w;
    windowHeight = h;
    setUniforms();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitContextVersion(3, 2);
    glutInitWindowSize (windowWidth, windowHeight);
    glutCreateWindow ("Voxel count");
    glutRepeatingTimerFunc(20);
    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    init();
    glutMainLoop();
}
