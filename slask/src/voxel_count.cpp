
#include "common_ext.hpp"
#include "programs/programs.hpp"
#include <iostream>

glm::mat4 viewFromWorld; 
glm::mat4 projectionMatrix;
glm::vec3 lightOrg;

Model *model;

gtl::shader::Phong phongShader;
gtl::shader::Hair hairShader;
gtl::shader::Texture3D textureShader;
gtl::shader::VoxelCount voxelCountShader;

GLfloat curTime;
GLfloat angle;

int windowWidth = 640;
int windowHeight = 480;

FBOstruct screenFBO;
FBOstruct voxelFBO;

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

int curLayer = 0;
int numLayers = 40;

void setUniforms() {
    glViewport(0, 0, windowWidth, windowHeight); 
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    projectionMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 1.25f, 2.75f);
    phongShader.use();
    phongShader.projectionMatrix = &projectionMatrix[0][0];
    hairShader.use();
    hairShader.projectionMatrix = &projectionMatrix[0][0];
}

void keyboardCallback(unsigned char event, int x, int y) {
    curLayer = 10 + static_cast<int>(event) - 48;
    std::cout << "curLayer = " << curLayer << std::endl;
}

void createFBO(FBOstruct & fbo, int width, int height, int depth) {

    fbo.width = width;
    fbo.height = height;

    // create objects
    glGenFramebuffers(1, &fbo.fb); 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fb);
    glGenTextures(1, &fbo.texid);
    glBindTexture(GL_TEXTURE_3D, fbo.texid); 
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    for(int i = 0; i < depth; ++i) {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, fbo.texid, 0, i); 
    }

    checkFBOStatus();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init(void) {

    phongShader.init();
    hairShader.init();
    textureShader.init();
    voxelCountShader.init();

    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glLineWidth(1);

    initFBO(screenFBO, windowWidth, windowHeight);

    createFBO(voxelFBO, windowWidth, windowHeight, numLayers);

    quad = LoadDataToModel(
            quadVerts, NULL, quadTexCoords, NULL,
            quadInds, 4, 6);

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

    textureShader.use();
    textureShader.sampler = 0; 
}

void display(void)
{

    //glBindFramebuffer(GL_FRAMEBUFFER, screenFBO.fb);
    glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO.fb);
//    glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, voxelFBO.texid, 0, 0); 
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, voxelFBO.texid, 0);
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
    DrawModel(model, voxelCountShader, "inPosition", NULL, NULL); 

    /*
    phongShader.use();
    phongShader.modelViewMatrix = &modelViewMatrix[0][0]; 
    phongShader.light = &light[0];
    DrawModel(model, phongShader, "inPosition", "inNormal", "");
    */

    /*
    hairShader.use();
    hairShader.modelViewMatrix = &modelViewMatrix[0][0];
    hairShader.light = &light[0];
    DrawModel(model, hairShader, "inPosition", "inNormal", "");
    */

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    textureShader.use();
    textureShader.z_TexCoord = static_cast<float>(curLayer) / numLayers;
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, screenFBO.texid);
    glBindTexture(GL_TEXTURE_3D, voxelFBO.texid);
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
    glutCreateWindow ("Hairy ball");
    glutRepeatingTimerFunc(20);
    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    init();
    glutMainLoop();
}
