
#include <iostream>
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "framebuffer.hpp"

glm::mat4 viewFromWorld; 
glm::mat4 modelViewMatrix;
glm::mat4 projectionMatrix;
glm::mat4 orthoMatrix;
glm::mat4 lightMatrix;
glm::vec3 lightOrg;
glm::vec3 light;

gtl::shader::HairDensity voxelCountShader;
gtl::shader::AddLayers cumsumShader;
//gtl::shader::Texture2DArray textureShader;
gtl::shader::HairRender textureShader;

GLfloat curTime;
GLfloat modelAngle;
GLfloat lightAngle;

GLsizei windowWidth = 640;
GLsizei windowHeight = 480;

gtl::Framebuffer<GL_TEXTURE_2D_ARRAY> voxelFBO;

Model * quad;
Model * ball;

int curLayer = 0;
int numLayers = 10;

void updateUniforms() {
    glViewport(0, 0, windowWidth, windowHeight); 
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    projectionMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 1.25f, 2.75f);
    orthoMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 0.5f, 3.5f);
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
    voxelFBO.texture.bind();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
    lightOrg = 2.0f * glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

    updateUniforms();

    textureShader.use();
}

void updateTime() {
    curTime = glutGet(GLUT_ELAPSED_TIME);	
    modelAngle = 0; //0.001f * curTime; 
    lightAngle = 0.001f * curTime;

    glm::mat4 worldFromModel = glm::rotate(modelAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    modelViewMatrix = viewFromWorld * worldFromModel;

    glm::mat3 lightRotation = glm::mat3(glm::rotate(lightAngle, glm::vec3(0.0f, 1.0f, 0.0f)));
    light = lightRotation * lightOrg; 
    lightMatrix = orthoMatrix * glm::lookAt(
            light,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    //lightMatrix = projectionMatrix * modelViewMatrix;
}

void drawDensity() {
    voxelFBO.bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_ONE, GL_ONE);
    voxelCountShader.use();
    voxelCountShader.projectionMatrix = &projectionMatrix[0][0];
    voxelCountShader.modelViewMatrix = &modelViewMatrix[0][0];
    voxelCountShader.numLayers = numLayers;
    DrawModel(ball, voxelCountShader, "in_Position", "in_Normal", NULL);
    /*
    drawModel(ball, voxelCountShader,
            &voxelCountShader.v_Position,
            &voxelCountShader.v_Normal,
            NULL);
    */
}

void cumsum() {
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
}

void drawHair() {
    voxelFBO.unbind();
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    voxelFBO.texture.bind(0);
    textureShader.use();
    textureShader.sampler = voxelFBO.texture.unit(); 
    textureShader.numLayers = numLayers;
    textureShader.projectionMatrix = &projectionMatrix[0][0];
    textureShader.modelViewMatrix = &modelViewMatrix[0][0];
    textureShader.lightMatrix = &lightMatrix[0][0];
    textureShader.light = &light[0];
    //textureShader.layer = curLayer;
    //DrawModel(quad, textureShader, "in_Position", "in_Normal", NULL);
    DrawModel(ball, textureShader, "in_Position", "in_Normal", NULL);
}

void display() {

    updateTime();
    drawDensity();
    cumsum();
    drawHair();

    printError("display");
    glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h) {
    windowWidth = w;
    windowHeight = h;
    updateUniforms();
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
