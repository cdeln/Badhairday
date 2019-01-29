
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
glm::mat4 modelLightMatrix;
glm::vec3 lightOrg;
glm::vec3 light;

gtl::shader::HairDensity hairDensityShader;
gtl::shader::AddLayers cumsumShader;
gtl::shader::HairRender hairRenderShader;

gtl::shader::Phong phongShader;

GLfloat curTime;
GLfloat modelAngle;
GLfloat lightAngle;

GLsizei windowWidth = 640;
GLsizei windowHeight = 480;

gtl::Framebuffer<GL_TEXTURE_3D> voxelFBO;

Model * quad;
Model * ball;

int curLayer = 0;
int numLayers = 10;

mat4 viewMatrix;

void updateUniforms() {
    glViewport(0, 0, windowWidth, windowHeight);
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    projectionMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 1.0f, 100.0f);
    orthoMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 0.5f, 3.5f);
}

void keyboardCallback(unsigned char event, int x, int y) {
    curLayer = static_cast<int>(event) - 48;
    std::cout << "curLayer = " << curLayer << std::endl;
}

void init(void) {

    hairRenderShader.init();
    cumsumShader.init();
    hairDensityShader.init();
    phongShader.init();

    voxelFBO.init();
    voxelFBO.allocate(windowWidth, windowHeight, numLayers);
    voxelFBO.texture.bind();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glLineWidth(1);

    quad = util::quad();
    ball = util::ball(0.5, 2);

    viewFromWorld = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    lightOrg = 2.0f * glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

    updateUniforms();

    vec3 cam = SetVector(0, 0, 2);
    vec3 point = SetVector(0, 0, 0);
    zprInit(&viewMatrix, cam, point);  // camera controls
}

void updateTime() {
    curTime = glutGet(GLUT_ELAPSED_TIME);
    modelAngle = 0 * curTime; //0.001f * curTime;
    lightAngle = 0 * curTime;

    viewFromWorld = glm::transpose(glm::make_mat4(&viewMatrix.m[0]));

    glm::mat4 worldFromModel = glm::rotate(modelAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    modelViewMatrix = viewFromWorld * worldFromModel;

    glm::mat3 lightRotation = glm::mat3(glm::rotate(lightAngle, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat3 normalMatrix = glm::mat3(viewFromWorld);
    light = normalMatrix * lightRotation * lightOrg;
    lightMatrix = glm::lookAt(
            light,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    modelLightMatrix = lightMatrix * worldFromModel;
}

void drawDensity() {
    voxelFBO.bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_ONE, GL_ONE);
    hairDensityShader.use();
    hairDensityShader.projectionMatrix = &orthoMatrix[0][0];
    hairDensityShader.modelViewMatrix = &modelLightMatrix[0][0];
    hairDensityShader.numLayers = numLayers;
    DrawModel(ball, hairDensityShader, "in_Position", "in_Normal", NULL);
    /*
    drawModel(ball, hairDensityShader,
            &hairDensityShader.v_Position,
            &hairDensityShader.v_Normal,
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

void drawBall() {
    voxelFBO.unbind();
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    phongShader.use();
    phongShader.modelViewMatrix = &modelViewMatrix[0][0];
    phongShader.projectionMatrix = &projectionMatrix[0][0];
    phongShader.light = &light[0];
    DrawModel(quad, phongShader, "inPosition", "inNormal", NULL);
}

void drawHair() {
    voxelFBO.unbind();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    voxelFBO.texture.bind(0);
    hairRenderShader.use();
    hairRenderShader.sampler = voxelFBO.texture.unit();
    hairRenderShader.numLayers = numLayers;
    hairRenderShader.projectionMatrix = &projectionMatrix[0][0];
    hairRenderShader.modelViewMatrix = &modelViewMatrix[0][0];
    glm::mat4 modelLightProjMatrix = orthoMatrix * modelLightMatrix;
    hairRenderShader.lightMatrix = &modelLightProjMatrix[0][0];
    hairRenderShader.light = &light[0];
    //hairRenderShader.layer = curLayer;
    //DrawModel(quad, hairRenderShader, "in_Position", "in_Normal", NULL);
    DrawModel(ball, hairRenderShader, "in_Position", "in_Normal", NULL);
}

void display() {

    bool hair = true;
    updateTime();
    if( hair ) {
    drawDensity();
    cumsum();
    voxelFBO.unbind();
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawHair();

    }
    else {
    drawBall();
    }

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
