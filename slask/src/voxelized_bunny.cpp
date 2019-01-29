
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
gtl::shader::HairTexDensity hairDensityShader;
gtl::shader::BunnyDensity bunnyDensityShader;
gtl::shader::AddLayers cumsumShader;
gtl::shader::HairTexLightRender hairRenderShader;
gtl::shader::LightVisualize lightVisualizeShader;
gtl::shader::Wireframe wireframeShader;
gtl::shader::VoxelRender voxelRender;
gtl::shader::Body phongShader;
gtl::shader::SimpleDensity densityShader;
gtl::shader::MeshDensity meshDensityShader;
//gtl::shader::HairVoxelizer hairVoxelizer;
gtl::shader::HairTessVoxelizer hairVoxelizer;
gtl::shader::Voxelizer voxelizer;
gtl::shader::VoxelClear voxelClear;
gtl::shader::LockClear lockClear;
gtl::shader::VoxelCumsum voxelCumsum;

// Framebuffers
gtl::Framebuffer<GL_TEXTURE_3D> voxelFBO;
GLuint voxelGrid;
GLuint voxelLock;

// Cameras
Camera cam;
Camera light;

// Models
Model * quad;
Model * cube;
Model * ball;
Model * lightBall;
Model * lattice;

// Texture
gtl::Texture<GL_TEXTURE_2D> hairTex;

// Globals
GLfloat lastTime;
GLsizei windowWidth = 640;
GLsizei windowHeight = 480;
int numLayers = 100;
GLsizei fboWidth = numLayers;
GLsizei fboHeight = numLayers;
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
int tessLevel = 3;
bool forwardInterpolate = false;
bool backwardInterpolate = true;
bool displayWireframe = false;
bool displayMeshDensity = false;
bool samplePerVertex = false;
bool doFloor = true;
bool doCumsum = true;
bool doFlip = true;
bool enableHair = true;
glm::vec3 ballPos;

glm::vec3 modelPos;
glm::mat4 modelMatrix;
float modelStepSize = 0.01;

// Random
Timer timer;
std::vector<GLfloat> voxels;
int voxelReadCounter = 0;

enum CURRENT_CAMERA {
    MAIN_CAMERA,
    LIGHT_CAMERA,
};

enum DISPLAY_MODE {
    DEBUG,
    RELEASE,
};

CURRENT_CAMERA currentCamera = MAIN_CAMERA;
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

void switchCamera() {
    switch(currentCamera) {
        case MAIN_CAMERA: currentCamera = LIGHT_CAMERA; break;
        case LIGHT_CAMERA: currentCamera = MAIN_CAMERA; break;
    }
}

void switchDisplayMode() {
    switch(displayMode) {
        case DEBUG: displayMode = RELEASE; break;
        case RELEASE: displayMode = DEBUG; break;
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
}

void updateTime() {
    float curTime = glutGet(GLUT_ELAPSED_TIME);
    //float dt = 0; //(curTime - lastTime) / 1000;
    //lightYaw += dt * 10;
    light.setRot(lightPitch, lightYaw);
    lastTime = curTime;

    ballPos = glm::vec3(2*sin(0.001*curTime), 0, 2*cos(0.001*curTime));

    voxelFBO.texture.bind();
    if( backwardInterpolate ) {
        voxelFBO.texture.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        voxelFBO.texture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else {
        voxelFBO.texture.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        voxelFBO.texture.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    modelMatrix = glm::translate(modelPos);
}

void init(void) {

    gtl::debug::init();

    // Init shaders
    hairRenderShader.init();
    cumsumShader.init();
    hairDensityShader.init();
    phongShader.init();
    lightVisualizeShader.init();
    wireframeShader.init();
    voxelRender.init();
    densityShader.init();
    bunnyDensityShader.init();
    meshDensityShader.init();
    hairVoxelizer.init();
    voxelizer.init();
    voxelClear.init();
    lockClear.init();
    voxelCumsum.init();

    // Init framebuffers
    voxelFBO.init();
    voxelFBO.texture.format(GL_FLOAT);
    voxelFBO.allocate(fboWidth, fboHeight, numLayers);
    voxelFBO.texture.bind(0);
    voxelFBO.texture.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    voxelFBO.texture.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    voxelFBO.texture.parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Init voxel grid
    glGenTextures(1, &voxelGrid);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, numLayers, numLayers, numLayers);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindImageTexture(0, voxelGrid, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);


    std::vector<GLuint> lockInit(numLayers * numLayers * numLayers, 0);
    glGenTextures(1, &voxelLock);
    glBindTexture(GL_TEXTURE_3D, voxelLock);
    //glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, numLayers, numLayers, numLayers);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, numLayers, numLayers, numLayers, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &lockInit[0]);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindImageTexture(1, voxelLock, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    //voxelFBO.texture.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //voxelFBO.texture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
    PRINTLN("Loading lattice");
    //lattice = util::model("models/lattice.obj");
    lattice = util::lattice(numLayers);
    PRINTLN("Loading ball");
    //ball = util::ball(0.5, 0);
    ball = util::model("models/ogasvans2fix.obj");
    ScaleModel(ball, 0.5, 0.5, 0.5);
    lightBall = util::ball(0.25, 2);

    //Load & bind texture
    LoadTGATextureSimple((char *)"textures/ogasvans.tga", &hairTex.texID);

    // GL stuff
    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glLineWidth(1);

    // Blir error
    //glPatchParameteri(GL_PATCH_VERTICES, 3);
    //glPatchParameteri(GL_PATCH_DEFAULT_OUTER_LEVEL, 3);
    //glPatchParameteri(GL_PATCH_DEFAULT_INNER_LEVEL, 3);

    // Globals
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    ballPos = glm::vec3(2,0,2);
    // Random
    voxels.resize(windowWidth * windowHeight * numLayers * 4);

    voxelFBO.texture.bind(0);
    hairTex.bind(1);
}

void drawDensity() {
    timer.tic("drawDensity");
    hairDensityShader.use();
    glm::mat4 lightView = light.getView();
    glm::mat4 lightProj = light.getProj();
    glm::mat4 modelViewMatrix = lightView * modelMatrix;
    hairDensityShader.projectionMatrix = &lightProj[0][0];
    hairDensityShader.modelViewMatrix = &modelViewMatrix[0][0];
    hairDensityShader.numLayers = numLayers;
    hairDensityShader.hairLength = hairLength;
    hairDensityShader.tex = hairTex;
    hairDensityShader.forceCenter = &ballPos[0];
    hairDensityShader.tessLevel = tessLevel;
    hairDensityShader.doFloor = doFloor;
    hairDensityShader.forwardInterpolate = forwardInterpolate;
    DrawModelPatches(ball, hairDensityShader, "inPosition", "inNormal", "inTexCoord");
    timer.toc("drawDensity");
}

//Draw density funktionen. Tänkt att ta in modellen och skriva i 3D-texturen
//med hög densitet.
void drawBodyDensity() {
    timer.tic("drawBodyDensity");

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    bunnyDensityShader.use();
    glm::mat4 lightView = light.getView();
    glm::mat4 lightProj = light.getProj();
    glm::mat4 modelViewMatrix = lightView  * modelMatrix;
    bunnyDensityShader.projectionMatrix = &lightProj[0][0];
    bunnyDensityShader.modelViewMatrix = &modelViewMatrix[0][0];
    bunnyDensityShader.numLayers = numLayers;
    bunnyDensityShader.tessLevel = tessLevel;
    bunnyDensityShader.forwardInterpolate = forwardInterpolate;
    DrawModelPatches(ball, bunnyDensityShader, "in_Position", NULL, NULL);

    timer.toc("drawBodyDensity");
}

void cumsum() {
    timer.tic("cumsum");
    cumsumShader.use();
    cumsumShader.sampler = voxelFBO.texture;
    cumsumShader.numLayers = numLayers;
    for(int i = 0; i < numLayers - 1; ++i) {
        cumsumShader.inputLayer = i + 0;
        cumsumShader.outputLayer = i + 1;
        DrawModel(quad, cumsumShader, "inPosition", NULL, "inTexCoord");
        glFinish();
    }
    /*
       glGenerateMipmap(GL_TEXTURE_3D);
       glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
       glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
       */
    timer.toc("cumsum");
}

void drawBall() {
    timer.tic("drawBall");

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
        //phongShader.sampler = voxelFBO.texture.unit();
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, voxelGrid);
        phongShader.sampler = 2;
        DrawModelPatches(ball, phongShader, "in_Position", "in_Normal", NULL);
    }

    glm::mat4 transform = cam.get();
    transform = transform * modelMatrix;
    if( displayWireframe ) {
        wireframeShader.use();
        wireframeShader.transform = &transform[0][0];
        DrawModel(ball, wireframeShader, "in_Position", NULL, NULL);
    }

    if( displayMeshDensity ) {
        meshDensityShader.use();
        meshDensityShader.transform = &transform[0][0];
        meshDensityShader.tessLevel = tessLevel;
        DrawModelPatches(ball, meshDensityShader, "in_Position", NULL, NULL);
    }

    timer.toc("drawBall");
}

void drawHair() {
    timer.tic("drawHair");

    hairRenderShader.use();
    hairRenderShader.sampler = 2; //voxelFBO.texture.unit();
    hairRenderShader.numLayers = numLayers;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    if( currentCamera == MAIN_CAMERA ) {
        viewMatrix = cam.getView();
        projectionMatrix = cam.getProj();
    }
    else if( currentCamera == LIGHT_CAMERA ) {
        viewMatrix = light.getView();
        projectionMatrix = light.getProj();
    }
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
    hairRenderShader.tessLevel = tessLevel;
    hairRenderShader.samplePerVertex = samplePerVertex;
    DrawModelPatches(ball, hairRenderShader, "in_Position", "in_Normal", "in_TexCoord");

    timer.toc("drawHair");
}

void drawLight() {
    timer.tic("drawLight");

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

    timer.toc("drawLight");
}

void voxelize() {
    timer.tic("voxelize");

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    timer.tic("voxelClear");
    voxelClear.use();
    glDispatchCompute(numLayers, numLayers, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glFinish();
    timer.toc("voxelClear");

    /*
    timer.tic("lockClear");
    lockClear.use();
    glDispatchCompute(numLayers, numLayers, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glFinish();
    timer.toc("lockClear");
    */


    glm::mat4 transform = light.get() * modelMatrix;
    /*
    timer.tic("Voxelize body");
    voxelizer.use();
    voxelizer.transform = &transform[0][0];
    DrawModel(ball, voxelizer, "in_Position", NULL, NULL);
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glFinish();
    timer.toc("Voxelize body");
    */

    timer.tic("Voxelize hair");
    hairVoxelizer.use();
    hairVoxelizer.transform = &transform[0][0];
    hairVoxelizer.tessLevel = tessLevel;
    hairVoxelizer.tex = hairTex;
    hairVoxelizer.forceCenter = &ballPos[0];
    hairVoxelizer.doFlip = doFlip;
    DrawModelPatches(ball, hairVoxelizer, "in_Position", "in_Normal", "in_TexCoord");
    //glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glFinish();
    timer.toc("Voxelize hair");

    timer.tic("voxelCumsum");
    if( doCumsum ) {
        voxelCumsum.use();
        glDispatchCompute(numLayers, numLayers, 1);
        //glMemoryBarrier(GL_ALL_BARRIER_BITS);
        //glFinish();
    }
    timer.toc("voxelCumsum");

    timer.toc("voxelize");
}

void drawVoxels() {
    timer.tic("drawVoxels");

    glm::mat4 lightVP = light.get();
    glm::mat4 lightVPInv = glm::inverse(lightVP);
    glm::mat4 cameraTransform = cam.get();
    glm::mat4 transform = cameraTransform * lightVPInv;

    voxelRender.use();
    voxelRender.transform = &transform[0][0];
    voxelRender.colorGain = voxelRenderColorGain;
    voxelRender.alphaGain = voxelRenderAlphaGain;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, voxelGrid);
    voxelRender.sampler = 2;
    DrawPointModel(lattice, voxelRender, "in_Position");

    timer.toc("drawVoxels");
}

void displayDebug() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);

    /*
       voxelFBO.bind();
       glViewport(0,0,fboWidth,fboHeight);
       glClearColor(0, 0, 0, 0);
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       */
    //voxelFBO.texture.bind();
    //glBindImageTexture(0, voxelFBO.texture.id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    //drawDensity();
    //drawBodyDensity();
    //cumsum();

    voxelize();

    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    drawVoxels();
    drawLight();
}

void displayRelease() {
    /*
       glDisable(GL_CULL_FACE);
       glDisable(GL_DEPTH_TEST);
       glBlendFunc(GL_ONE, GL_ONE);

       voxelFBO.bind();
       glViewport(0,0,fboWidth,fboHeight);
       glClearColor(0, 0, 0, 0);
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       */
    //drawDensity();
    //drawBodyDensity();
    //cumsum();

    //voxelFBO.unbind();
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_CULL_FACE);
    glViewport(0,0,numLayers,numLayers);
    voxelize();

    glViewport(0,0,windowWidth,windowHeight);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    drawBall();
    if( enableHair )  {
        drawHair();
    }
    //drawLight();

}

void display() {

    timer.tic("display");

    updateTime();
    switch( displayMode ) {
        case DEBUG: displayDebug(); break;
        case RELEASE: displayRelease(); break;
    }

    bool saveTexture = false;
    if( saveTexture ) {
        if( voxelReadCounter == 0 ) {
            std::cout << "reading pixels" << std::endl;
            glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, &voxels[0]);
            std::cout << "pixels read" << std::endl;
            std::cout << "voxels size = " << windowWidth * windowHeight * numLayers * 4<< " / " << voxels.size() << std::endl;
            std::cout << "should write " << sizeof(float) * voxels.size() << std::endl;
            std::ofstream file;
            file.open("data.dat", std::ios::binary);
            file.write((char*)&voxels[0], sizeof(float) * voxels.size());
            file.close();
            std::cout << "writen to file" << std::endl;
            voxelReadCounter = 1;
        }
    }

    printError("display");
    glutSwapBuffers();

    timer.toc("display");
    timer.report();
}

void reshape(GLsizei newWidth, GLsizei newHeight) {
    PRINTLN("Width: " << newWidth << ", Height: " << newHeight);
    windowWidth = newWidth;
    windowHeight = newHeight;
    voxelFBO.unbind();
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
    glutCreateWindow ("Furball");
    glutRepeatingTimerFunc(20);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    glutMotionFunc(mouseCallback);
    glutMouseFunc(clickCallback);
    init();
    glutMainLoop();
}
