
#include "common.h"
#include "programs/programs.hpp"
#include <iostream>

glm::mat4 viewFromWorld; 
glm::mat4 perpMatrix;
glm::mat4 orthoMatrix;
glm::mat4 projectionMatrix;
glm::vec3 lightOrg;

Model *model;

gtl::shader::Phong phongShader;
gtl::shader::Hair hairShader;
gtl::shader::Texture textureShader;

GLfloat curTime;
GLfloat angle;

int windowWidth = 640;
int windowHeight = 480;

FBOstruct * fbo;
GLfloat quadVerts[] = {
    -1,-1,0,
    1,-1, 0,
    1,1, 0,
    -1,1, 0};
GLfloat quadTexCoords[] = {
    0, 0,
    1, 0,
    1, 1,
    0, 1};
GLuint quadInds[] = {0, 1, 2, 0, 2, 3};
Model * quad;

int displayMode = 1;

void setUniforms() {
    glViewport(0, 0, windowWidth, windowHeight); 
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    GLfloat ratio = w / h; 
    perpMatrix = glm::perspective(45.0f, ratio, 1.0f, 300.0f);
    orthoMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 1.0f, 300.0f);
    if( displayMode == 1 ) {
        projectionMatrix = perpMatrix; 
    }
    else if( displayMode == 2 ) {
        projectionMatrix = orthoMatrix; 
    }
    phongShader.use();
    phongShader.projectionMatrix = &projectionMatrix[0][0];
    hairShader.use();
    hairShader.projectionMatrix = &projectionMatrix[0][0];
}

void keyboardCallback(unsigned char event, int x, int y)
{
  displayMode = static_cast<int>(event) - 48;
  setUniforms();
  std::cout << "displayMode = " << displayMode << std::endl;
}

void init(void) {

    phongShader.init();
    hairShader.init();
    textureShader.init();

    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glLineWidth(1);

    fbo = initFBO(windowWidth, windowHeight, 0);
    quad = LoadDataToModel(
            quadVerts, NULL, quadTexCoords, NULL,
            quadInds, 4, 6);

    projectionMatrix = glm::perspective(45.0f, 1.0f, 1.0f, 300.0f);
    viewFromWorld = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
    lightOrg = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

    setUniforms();

    model = LoadModelPlus("models/sphere2.obj");
    CenterModel(model);
    ScaleModel(model, 0.5, 0.5, 0.5);
    ReloadModelData(model);

    phongShader.use();
    phongShader.projectionMatrix = &projectionMatrix[0][0];

    hairShader.use();
    hairShader.projectionMatrix = &projectionMatrix[0][0];

    std::cout << "before use" << std::endl;
    textureShader.use();
    std::cout << "after use" << std::endl;
    textureShader.sampler = 0; //fbo->texid;
}

void display(void)
{

    useFBO(fbo, 0L, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curTime = glutGet(GLUT_ELAPSED_TIME);	
    angle = 0.001f * curTime; 

    glm::mat4 worldFromModel = glm::rotate(angle, glm::vec3(1.0f, 1.0f, 1.0f));
    glm::mat4 modelViewMatrix = viewFromWorld * worldFromModel;

    glm::mat3 lightRotation = glm::mat3(glm::rotate(2.3f*angle, glm::vec3(0.0f,1.0f,0.0f)));
    glm::vec3 light = lightRotation * lightOrg; 

    phongShader.use();
    phongShader.modelViewMatrix = &modelViewMatrix[0][0]; 
    phongShader.light = &light[0];
    DrawModel(model, phongShader, "inPosition", "inNormal", "");

    hairShader.use();
    hairShader.modelViewMatrix = &modelViewMatrix[0][0];
    hairShader.light = &light[0];
    DrawModel(model, hairShader, "inPosition", "inNormal", "");

    useFBO(0L, fbo, 0L);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    textureShader.use();
    DrawModel(quad, textureShader, "inPosition", NULL, "inTexCoord");

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
    glutCreateWindow ("Hairy ball");
    glutRepeatingTimerFunc(20);
    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    init();
    glutMainLoop();
}
