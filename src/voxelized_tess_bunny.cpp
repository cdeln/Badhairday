
#include <iostream>
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "framebuffer.hpp"
#include "LoadTGA.h"
#include "macros.h"
#include "camera.hpp"
#include "timer.hpp"

// Shaders
gtl::shader::HairTessVoxelizer hairVoxelizer;
gtl::shader::HairTessRenderer hairRenderShader;
gtl::shader::VoxelClear voxelClear;
gtl::shader::VoxelCumsum voxelCumsum;

gtl::shader::LightVisualize lightVisualizeShader;
gtl::shader::Wireframe wireframeShader;

gtl::shader::Body phongShader;
gtl::shader::MeshDensity meshDensityShader;

gtl::shader::DepthRenderer depthRenderer;
gtl::shader::QuadRenderer quadRenderer;

// Framebuffers
GLuint fbo;

// Textures
GLuint voxelGrid;
GLuint depthTex;
gtl::Texture<GL_TEXTURE_2D> hairTex;

// Cameras
Camera cam;
Camera light;

// Models
Model * quad;
Model * cube;
Model * model; 
Model * lightBall;

// Timer 
Timer timer;
#define tic(x) timer.tic(x)
#define toc(x) glFinish(); timer.toc(x); 

// Globals
GLsizei windowWidth = 640;
GLsizei windowHeight = 480;
GLsizei depthWidth = 512; //1024;
GLsizei depthHeight = 512; //1024;
int numLayers = 100;
float voxelRenderColorGain = 0.1f;
float voxelRenderAlphaGain = 0.2f;
float voxelRenderGainStep = 0.01f;
float clickX = 0;
float clickY = 0;
float lastPitch = 0;
float lastYaw = 0;
float prevX = 0;
float prevY = 0;
float pitch;
float yaw;
float camDist = 5;
float camDistStep = 1.1;
float FOV = 45 * M_PI / 180.0;
float hairLength = 0.4;
float shadeFactor = 5.0;
float lightYaw = 0;
float lightPitch = 0;
int tessLevel = 15;
bool forwardInterpolate = false;
bool backwardInterpolate = true;
bool displayWireframe = false;
bool displayMeshDensity = false;
bool samplePerVertex = false;
bool doFloor = true;
bool doCumsum = true;
bool doFlip = true;
bool enableHair = true;
bool enableShadowMap = true;
glm::vec3 ballPos;

glm::vec3 modelPos;
glm::mat4 modelMatrix;
float modelStepSize = 0.01;

enum DISPLAY_MODE {
    DEBUG,
    DEPTH,
    RELEASE,
};

DISPLAY_MODE displayMode = RELEASE;

void mouseCallback(int x, int y) {
    PRINT("mouseCallback"); VERBOSE(2,x,y);
    float mx = 2 * (static_cast<float>(x) / windowWidth - 0.5);
    float my = 2 * (static_cast<float>(y) / windowHeight - 0.5);
    mx -= clickX;
    my -= clickY;
    prevX = mx;
    prevY = my;
    pitch = lastPitch + 100 * my;
    yaw = lastYaw + 100 * mx;
    pitch = util::clamp(pitch, -89, 89);
    cam.setRot(- pitch, - yaw);
    VERBOSE(2, pitch, yaw);
}

void clickCallback(int button, int status, int x, int y) {
    PRINT("clickCallback: "); VERBOSE(4,button, status, x, y);
    if( status == GLUT_DOWN ) {
        float mx = 2 * (static_cast<float>(x) / windowWidth - 0.5);
        float my = 2 * (static_cast<float>(y) / windowHeight - 0.5);
        prevX = 0;
        prevY = 0;
        clickX = mx;
        clickY = my;
    }
    else if ( status == GLUT_UP ) {
        lastPitch = pitch;
        lastYaw = yaw;
    }
    if( button == GLUT_SCROLL ) {
        PRINTLN("Pressed scroll");
        if( status == GLUT_DOWN ) {
            camDist *= camDistStep;
        }
        else if ( status == GLUT_UP ) {
            camDist /= camDistStep;
        }
        camDist = util::clamp(camDist, 0.1, 10);
        cam.setDist(camDist);
    }
}

void toggleCamViewMode() {
    switch(cam.viewMode()) {
        case Camera::ViewMode::PIVOT: cam.setViewMode(Camera::ViewMode::FREE); break;
        case Camera::ViewMode::FREE: cam.setViewMode(Camera::ViewMode::PIVOT); break;
    }
}

void switchDisplayMode() {
    switch(displayMode) {
        case DEPTH: displayMode = RELEASE; break;
        case RELEASE: displayMode = DEPTH; break;
    }
}

void keyboardCallback(unsigned char key, int x, int y) {
    if( key == '1' ) {
        voxelRenderColorGain -= voxelRenderGainStep;
        VERBOSE(2, voxelRenderColorGain, voxelRenderAlphaGain);
    }
    else if ( key == '2' ) {
        voxelRenderColorGain += voxelRenderGainStep;
        VERBOSE(2, voxelRenderColorGain, voxelRenderAlphaGain);
    }
    if( key == '3' ) {
        voxelRenderAlphaGain -= voxelRenderGainStep;
        VERBOSE(2, voxelRenderColorGain, voxelRenderAlphaGain);
    }
    else if ( key == '4' ) {
        voxelRenderAlphaGain += voxelRenderGainStep;
        VERBOSE(2, voxelRenderColorGain, voxelRenderAlphaGain);
    }
    else if ( key == '5' ) {
        shadeFactor = std::max(shadeFactor - 0.1, 0.0);
        VERBOSE(1, shadeFactor);
    }
    else if ( key == '6' ) {
        shadeFactor = std::max(shadeFactor + 0.1, 0.0);
        VERBOSE(1, shadeFactor);
    }
    else if ( key == 'q' ) {
        toggleCamViewMode();
    }
    else if ( key == 'w' ) {
        //cam.moveLocal(0,0,-0.1);
        modelPos.z -= modelStepSize;
    }
    else if (key == 's' ) {
        //cam.moveLocal(0,0,0.1);
        modelPos.z += modelStepSize;
    }
    else if ( key == 'a' ) {
        //cam.moveLocal(-0.1,0,0);
        modelPos.x -= modelStepSize;
    }
    else if ( key == 'd' ) {
        //cam.moveLocal(0.1,0,0);
        modelPos.x += modelStepSize;
    }
    else if ( key == 'c' ) {
        doCumsum = ! doCumsum;
        VERBOSE(1, doCumsum);
    }
    else if ( key == 'x' ) {
        switchDisplayMode();
    }
    else if ( key == '+' ) {
        tessLevel += 1;
        VERBOSE(1,tessLevel);
    }
    else if ( key == '-' ) {
        tessLevel = std::max(tessLevel - 1, 1);
        VERBOSE(1,tessLevel);
    }
    else if ( key == 'o' ) {
        lightYaw -= 0.1;
    }
    else if ( key == 'p' ) {
        lightYaw += 0.1;
    }
    else if ( key == 'm' ) {
        displayWireframe = ! displayWireframe;
    }
    else if ( key == 'n' ) {
        displayMeshDensity = ! displayMeshDensity;
    }
    else if ( key == 'k' ) {
        forwardInterpolate = ! forwardInterpolate;
        VERBOSE(1, forwardInterpolate);
    }
    else if ( key == 'l' ) {
        backwardInterpolate = ! backwardInterpolate;
        VERBOSE(1, backwardInterpolate);
    }
    else if ( key == 'v' ) {
        samplePerVertex = ! samplePerVertex;
        VERBOSE(1, samplePerVertex);
    }
    else if ( key == 'f' ) {
        doFloor = ! doFloor;
        VERBOSE(1,doFloor);
    }
    else if( key == 'g' ) {
        doFlip = ! doFlip;
    }
    else if (key == 'h' ) {
        enableHair = ! enableHair;
    }
    else if ( key == 'b' ) {
        enableShadowMap = ! enableShadowMap;
    }
}

void updateTime() {
    float curTime = glutGet(GLUT_ELAPSED_TIME);
    light.setRot(lightPitch, lightYaw);

    ballPos = glm::vec3(2*sin(0.001*curTime), 0, 2*cos(0.001*curTime));

    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    if( backwardInterpolate ) {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    }
    glBindTexture(GL_TEXTURE_3D, 0);

    modelMatrix = glm::translate(modelPos);
}

void init(void) {

    // Enable OpenGL to throw errors at you
    gtl::debug::init();

    // Init shaders
    hairRenderShader.init();
    phongShader.init();
    lightVisualizeShader.init();
    wireframeShader.init();
    meshDensityShader.init();

    hairVoxelizer.init();
    voxelClear.init();
    voxelCumsum.init();

    depthRenderer.init();
    quadRenderer.init();

    //// Init textures
    // Init voxel grid
    glGenTextures(1, &voxelGrid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, numLayers, numLayers, numLayers);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindImageTexture(0, voxelGrid, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

    // Setup framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &depthTex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 1024, 1024); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depthWidth, depthHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    /*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
    glDrawBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw gtl::Exception("FRAMEBUFFER");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Init cameras
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    float aspect = w / h;
    cam.setPerspective(45 * M_PI / 180.0, aspect, 0.01, 100);
    cam.setViewMode(Camera::ViewMode::PIVOT);
    cam.setDist(5);
    GLM_VERBOSE(cam.getView());
    GLM_VERBOSE(cam.getProj());

    light.setViewMode(Camera::ViewMode::PIVOT);
    light.setProjMode(Camera::ProjMode::ORTHO);
    light.setPos(1.5,0,0);
    light.lookAt(0,0,0);
    light.setWidth(3);
    light.setHeight(3);
    light.setDepth(0, 3);

    modelPos = glm::vec3(0,0,0);

    // Load models
    quad = util::quad();
    cube = util::cube();
    model = util::model("models/ogasvans2.obj");
    ScaleModel(model, 0.5, 0.5, 0.5);
    lightBall = util::ball(0.25, 2);

    // Load textures
    LoadTGATextureSimple((char *)"textures/ogasvans.tga", &hairTex.texID);
    hairTex.bind(1);

    // GL stuff
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glLineWidth(1);

    // Globals
    ballPos = glm::vec3(2,0,2);
}

void drawDepth() {
    tic("drawDepth");

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_CULL_FACE);
    glViewport(0,0,depthWidth,depthHeight);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    depthRenderer.use();
    depthRenderer.transform = light.get();
    DrawModel(model, depthRenderer, "in_Position", NULL, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    toc("drawDepth");
}

void displayDepth() {
    tic("displayDepth");

    drawDepth();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_CULL_FACE);
    glViewport(0,0,windowWidth,windowHeight);

    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    quadRenderer.use();
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    quadRenderer.sampler = 2; // depth texture
    DrawModel(quad, quadRenderer, "in_Position", NULL, "in_TexCoord"); 
    glBindTexture(GL_TEXTURE_2D, 0);

    toc("displayDepth");
}

void drawModel() {
    tic("drawBall");

    if( ! displayMeshDensity ) {
        phongShader.use();
        glm::mat4 viewMatrix = cam.getView();
        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat4 projectionMatrix = cam.getProj();
        glm::mat4 lightMatrix = light.get() * modelMatrix;
        glm::vec3 lightPos = glm::mat3(modelViewMatrix) * light.getPos();
        phongShader.modelViewMatrix = &modelViewMatrix[0][0];
        phongShader.projectionMatrix = &projectionMatrix[0][0];
        phongShader.lightMatrix = &lightMatrix[0][0];
        phongShader.light = &lightPos[0];
        phongShader.tessLevel = tessLevel;
        phongShader.numLayers = numLayers;
        phongShader.shadeFactor = shadeFactor;
        phongShader.tex = hairTex;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, voxelGrid);
        phongShader.sampler = 0;
        DrawModelPatches(model, phongShader, "in_Position", "in_Normal", NULL);
    }

    glm::mat4 transform = cam.get();
    transform = transform * modelMatrix;
    if( displayWireframe ) {
        wireframeShader.use();
        wireframeShader.transform = &transform[0][0];
        DrawModel(model, wireframeShader, "in_Position", NULL, NULL);
    }

    if( displayMeshDensity ) {
        meshDensityShader.use();
        meshDensityShader.transform = &transform[0][0];
        meshDensityShader.tessLevel = tessLevel;
        DrawModelPatches(model, meshDensityShader, "in_Position", NULL, NULL);
    }

    toc("drawBall");
}

void drawHair() {
    tic("drawHair");

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    hairRenderShader.use();
    hairRenderShader.sampler = 0; 
    hairRenderShader.numLayers = numLayers;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    viewMatrix = cam.getView();
    projectionMatrix = cam.getProj();
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    hairRenderShader.projectionMatrix = &projectionMatrix[0][0];
    hairRenderShader.modelViewMatrix = &modelViewMatrix[0][0];
    glm::mat4 lightVP = light.get(); //lightProj * lightView;
    lightVP = lightVP * modelMatrix;
    hairRenderShader.lightMatrix = &lightVP[0][0];
    glm::vec3 lightPos = glm::mat3(modelViewMatrix) * light.getPos();
    hairRenderShader.light = &lightPos[0];
    hairRenderShader.hairLength = hairLength;
    hairRenderShader.shadeFactor = shadeFactor;
    hairRenderShader.forceCenter = &ballPos[0];
    hairRenderShader.tex = hairTex;
    hairRenderShader.depthSampler = 2; // depth texture
    hairRenderShader.tessLevel = tessLevel;
    hairRenderShader.samplePerVertex = samplePerVertex;
    hairRenderShader.enableShadowMap = enableShadowMap;
    DrawModelPatches(model, hairRenderShader, "in_Position", "in_Normal", "in_TexCoord");

    toc("drawHair");
}

void drawLight() {
    tic("drawLight");

    lightVisualizeShader.use();
    glm::mat4 viewFromWorld = cam.getView();
    glm::vec3 lightPos = light.getPos();
    glm::mat4 worldFromModel = glm::translate(lightPos);
    glm::mat4 lightModelViewMatrix = viewFromWorld * worldFromModel;
    glm::mat4 projectionMatrix = cam.getProj();
    lightVisualizeShader.modelViewMatrix = &lightModelViewMatrix[0][0];
    lightVisualizeShader.projectionMatrix = &projectionMatrix[0][0];
    DrawModel(lightBall, lightVisualizeShader, "in_Position", NULL, NULL);

    wireframeShader.use();
    glm::mat4 lightVP = light.get();
    glm::mat4 lightVPInv = glm::inverse(lightVP);
    glm::mat4 cameraTransform = cam.get();
    glm::mat4 transform = cameraTransform * lightVPInv;
    wireframeShader.transform = &transform[0][0];
    DrawModel(cube, wireframeShader, "in_Position", NULL, NULL);

    toc("drawLight");
}

void voxelize() {
    tic("voxelize");

    // Disable color and z-buffer rendering, set viewport
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_CULL_FACE);
    glViewport(0,0,numLayers,numLayers);

    // clear voxels
    tic("voxelClear");
    voxelClear.use();
    glDispatchCompute(numLayers, numLayers, 1);
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glFinish();
    toc("voxelClear");


    glm::mat4 transform = light.get() * modelMatrix;
    /*
    tic("Voxelize body");
    voxelizer.use();
    voxelizer.transform = &transform[0][0];
    DrawModel(ball, voxelizer, "in_Position", NULL, NULL);
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glFinish();
    toc("Voxelize body");
    */

    tic("Voxelize hair");
    hairVoxelizer.use();
    hairVoxelizer.transform = &transform[0][0];
    hairVoxelizer.tessLevel = tessLevel;
    hairVoxelizer.tex = hairTex;
    hairVoxelizer.forceCenter = &ballPos[0];
    hairVoxelizer.doFlip = doFlip;
    DrawModelPatches(model, hairVoxelizer, "in_Position", "in_Normal", "in_TexCoord");
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glFinish();
    toc("Voxelize hair");

    tic("voxelCumsum");
    if( doCumsum ) {
        voxelCumsum.use();
        glDispatchCompute(numLayers, numLayers, 1);
        //glMemoryBarrier(GL_ALL_BARRIER_BITS);
        //glFinish();
    }
    toc("voxelCumsum");

    toc("voxelize");
}

void drawVoxels() {
    // 
}

void displayDebug() {

    voxelize();

    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    drawVoxels();
    drawLight();
}

void displayRelease() {

    // Voxelize the scene
    voxelize();

    drawDepth();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Enable color and z-buffers, set viewport
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glViewport(0,0,windowWidth,windowHeight);

    // Clear the scene
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the scene
    drawModel();
    if( enableHair )  {
        drawHair();
    }
    drawLight();
}

void display() {

    tic("display");

    updateTime();
    switch( displayMode ) {
        case DEPTH: displayDepth(); break;
        case RELEASE: displayRelease(); break;
    }

    printError("display");
    glutSwapBuffers();

    toc("display");
    timer.report();
}

void reshape(GLsizei newWidth, GLsizei newHeight) {
    PRINTLN("Width: " << newWidth << ", Height: " << newHeight);
    windowWidth = newWidth;
    windowHeight = newHeight;
    float w = static_cast<float>(windowWidth);
    float h = static_cast<float>(windowHeight);
    float aspect = w / h;
    cam.setAspect(aspect);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitContextVersion(4, 5);
    glutInitWindowSize (windowWidth, windowHeight);
    glutCreateWindow ("Voxelization based shading of bezier patch tesselated bunny using texture based procedural hair geometry generation"); 
    glutRepeatingTimerFunc(20);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    glutMotionFunc(mouseCallback);
    glutMouseFunc(clickCallback);
    init();
    glutMainLoop();
}
