
#include <iostream>
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "framebuffer.hpp"
#include "LoadTGA.h"
#include "macros.h"
#include "camera.hpp"

// Shaders
gtl::shader::LightVisualize lightVisualizeShader;
gtl::shader::Wireframe wireframeShader;
gtl::shader::TessViz tessShader;
gtl::shader::NormalViz normalVizShader;
gtl::shader::TessWireframe tessWireframeShader;

// Framebuffers
gtl::Framebuffer<GL_TEXTURE_3D> voxelFBO;

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
int numLayers = 50;
GLsizei fboWidth = numLayers;
GLsizei fboHeight = numLayers;
float voxelRenderColorGain = 0.01f;
float voxelRenderAlphaGain = 0.01f;
float voxelRenderGainStep = 0.001f;
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
float shadeFactor = 1.75f;
float lightYaw = 0;
float lightPitch = 0;
int innerTessLevel = 2;
int outerTessLevel = 1;
bool drawNormals = false;
bool drawTesselation = false;

// Random
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
    }
    else if ( key == '2' ) {
        voxelRenderColorGain += voxelRenderGainStep;
    }
    if( key == '3' ) {
        voxelRenderAlphaGain -= voxelRenderGainStep;
    }
    else if ( key == '4' ) {
        voxelRenderAlphaGain += voxelRenderGainStep;
    }
    else if ( key == '5' ) {
        shadeFactor = std::max(shadeFactor - 0.01, 0.0);
    }
    else if ( key == '6' ) {
        shadeFactor = std::max(shadeFactor + 0.01, 0.0);
    }
    else if ( key == 'q' ) {
        toggleCamViewMode();
    }
    else if ( key == 'w' ) {
        cam.moveLocal(0,0,-0.1);
    }
    else if (key == 's' ) {
        cam.moveLocal(0,0,0.1);
    }
    else if ( key == 'a' ) {
        cam.moveLocal(-0.1,0,0);
    }
    else if ( key == 'd' ) {
        cam.moveLocal(0.1,0,0);
    }
    else if ( key == 'c' ) {
        switchCamera();
    }
    else if ( key == 'x' ) {
        switchDisplayMode();
    }
    else if ( key == '+' ) {
        innerTessLevel += 1;
        VERBOSE(1, innerTessLevel);
    }
    else if ( key == '-' ) {
        innerTessLevel = std::max(innerTessLevel - 1, 1);
        VERBOSE(1, innerTessLevel);
    }
    else if ( key == '9' ) {
        outerTessLevel += 1;
        VERBOSE(1, outerTessLevel);
    }
    else if ( key == '8' ) {
        outerTessLevel = std::max(outerTessLevel - 1, 1);
        VERBOSE(1, outerTessLevel);
    }
    else if( key == 'n') {
        drawNormals = ! drawNormals;
    }
    else if ( key == 'm' ) {
        drawTesselation = ! drawTesselation;
    }
    VERBOSE(3, voxelRenderColorGain, voxelRenderAlphaGain, shadeFactor);
}

void updateTime() {
    float curTime = glutGet(GLUT_ELAPSED_TIME);
    float dt = (curTime - lastTime) / 1000;
    lightYaw += dt * 10;
    light.setRot(lightPitch, lightYaw);
    lastTime = curTime;
}

void init(void) {

    // Init shaders
    tessShader.init();
    normalVizShader.init();
    tessWireframeShader.init();
    lightVisualizeShader.init();
    wireframeShader.init();

    // Init framebuffers
    voxelFBO.init();
    voxelFBO.allocate(fboWidth, fboHeight, numLayers);
    voxelFBO.texture.bind();
    voxelFBO.texture.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    voxelFBO.texture.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    voxelFBO.texture.parameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    voxelFBO.texture.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    voxelFBO.texture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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

    // Load models
    quad = util::quad();
    cube = util::cube();
    PRINTLN("Loading lattice");
    //lattice = util::model("models/lattice.obj");
    lattice = util::lattice(30);
    PRINTLN("Loading ball");
    //ball = util::ball(1.0, 3);
    //ball = util::model("models/longneck.obj");
    ball = util::model("models/ogasvans2fix.obj");
    ScaleModel(ball, 0.5, 0.5, 0.5);
    lightBall = util::ball(0.25, 2);

    //Load & bind texture
    LoadTGATextureSimple((char *)"textures/longneck.tga", &hairTex.texID);

    // GL stuff
    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glLineWidth(1);

    glPatchParameteri(GL_PATCH_DEFAULT_OUTER_LEVEL, 1);
    glPatchParameteri(GL_PATCH_DEFAULT_INNER_LEVEL, 1);
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Globals
    lastTime = glutGet(GLUT_ELAPSED_TIME);

    // Random
    voxels.resize(windowWidth * windowHeight * numLayers * 4);

    voxelFBO.texture.bind(0);
    hairTex.bind(1);
}

void drawBall() {

    glm::mat4 transform = cam.get();

    glBlendFunc(GL_ADD, GL_ADD);

    tessShader.use();
    tessShader.transform = &transform[0][0];
    tessShader.tessLevel = innerTessLevel;
    tessShader.outerTessLevel = outerTessLevel;
    DrawModelPatches(ball, tessShader, "in_Position", "in_Normal", NULL);

    if( drawNormals ) {
        normalVizShader.use();
        normalVizShader.normalLength = 0.1;
        normalVizShader.transform = &transform[0][0];
        normalVizShader.tessLevel = innerTessLevel;
        normalVizShader.outerTessLevel = outerTessLevel;
        DrawModelPatches(ball, normalVizShader, "in_Position", "in_Normal", NULL);
    }

	glDisable(GL_DEPTH_TEST);
    if( drawTesselation ) {
        tessWireframeShader.use();
        tessWireframeShader.transform = &transform[0][0];
        tessWireframeShader.tessLevel = innerTessLevel;
        tessWireframeShader.outerTessLevel = outerTessLevel;
        DrawModelPatches(ball, tessWireframeShader, "in_Position", NULL, NULL);
    }
	glEnable(GL_DEPTH_TEST);
}

void drawLight() {
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
}

void display() {

    updateTime();

    voxelFBO.unbind();
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    drawBall();
    printError("drawBall");

    printError("display");
    glutSwapBuffers();
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
    glutInitContextVersion(4, 2);
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
