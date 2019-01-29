
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include <iostream>

glm::mat4 viewFromWorld; 
glm::mat4 projectionMatrix;
glm::vec3 lightOrg;

Model *model;

gtl::shader::Texture2DArray textureShader;
gtl::shader::VoxelCount voxelCountShader;
gtl::shader::AddLayers cumsumShader;

GLfloat curTime;
GLfloat angle;

int windowWidth = 640;
int windowHeight = 480;

FBOstruct screenFBO;
FBOstruct voxelFBO;

Model * quad;

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

void createFBO(FBOstruct & fbo, int width, int height, int depth) {

    fbo.width = width;
    fbo.height = height;

    // create objects
    glGenFramebuffers(1, &fbo.fb); 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fb);
    glGenTextures(1, &fbo.texid);
    glBindTexture(GL_TEXTURE_2D_ARRAY, fbo.texid); 
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    printError("glTexImage3D");
    for(int i = 0; i < depth; ++i) {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo.texid, 0, i); 
    }
    printError("glFramebufferTextureLayer");

    checkFBOStatus();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init(void) {

    textureShader.init();
    voxelCountShader.init();
    cumsumShader.init();

    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glLineWidth(1);

    initFBO(screenFBO, windowWidth, windowHeight);

    createFBO(voxelFBO, windowWidth, windowHeight, numLayers);

    quad = util::quad();

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

    textureShader.use();
    textureShader.sampler = 0; 
}

void display(void)
{

    //glBindFramebuffer(GL_FRAMEBUFFER, screenFBO.fb);
    glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO.fb);
//    glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, voxelFBO.texid, 0, 0); 
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, voxelFBO.texid, 0);
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
    DrawModel(model, voxelCountShader, "inPosition", NULL, NULL); 

    // Cumsum
    glBindFramebuffer(GL_FRAMEBUFFER, voxelFBO.fb);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, voxelFBO.texid, 0);

    glBlendFunc(GL_ONE, GL_ONE);

    cumsumShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, voxelFBO.texid);
    cumsumShader.sampler = 0;
    for(int i = 0; i < numLayers - 1; ++i) {
        cumsumShader.inputLayer = i + 0;
        cumsumShader.outputLayer = i + 1;
        DrawModel(quad, cumsumShader, "inPosition", NULL, "inTexCoord");
    }


    // Draw onto screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    textureShader.use();
    textureShader.layer = curLayer;
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, screenFBO.texid);
    glBindTexture(GL_TEXTURE_2D_ARRAY, voxelFBO.texid);
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
