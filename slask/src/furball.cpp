
#include <iostream>
#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "framebuffer.hpp"
#include "macros.h"
#include "camera.hpp"

// Shaders
gtl::shader::HairDensity hairDensityShader;
gtl::shader::AddLayers cumsumShader;
gtl::shader::HairRender hairRenderShader;
gtl::shader::LightVisualize lightVisualizeShader;
gtl::shader::Wireframe wireframeShader;
gtl::shader::VoxelRender voxelRender;
gtl::shader::Phong phongShader;
gtl::shader::SimpleDensity densityShader;

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

// Globals
GLfloat lastTime; 
GLsizei windowWidth = 640;
GLsizei windowHeight = 480;
int numLayers = 20;
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
float pitchVel; 
float yawVel;
float camDist = 5;
float camDistStep = 1.1;
float FOV = 45 * M_PI / 180.0;
float hairLength = 0.1;
float shadeFactor = 0.8f;

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
DISPLAY_MODE displayMode;

void mouseCallback(int x, int y) {
    PRINT("mouseCallback"); VERBOSE(2,x,y);
    float mx = 2 * (static_cast<float>(x) / windowWidth - 0.5);
    float my = 2 * (static_cast<float>(y) / windowHeight - 0.5);
    mx -= clickX;
    my -= clickY;
    float dx = mx - prevX;
    float dy = my - prevY;
    prevX = mx;
    prevY = my;
    pitch = lastPitch + 100 * my;
    yaw = lastYaw + 100 * mx;
    pitch = util::clamp(pitch, -89, 89);
    pitchVel = 5000 * dy; 
    yawVel = 5000 * dx;
    //cam.setRotVel(pitchVel, yawVel);
    cam.setRot(- pitch, - yaw);
    VERBOSE(4, pitch, yaw, pitchVel, yawVel); 
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
    VERBOSE(3, voxelRenderColorGain, voxelRenderAlphaGain, shadeFactor);
}

void updateTime() {
    float curTime = glutGet(GLUT_ELAPSED_TIME);      
    lastTime = curTime;
    //float dt = (curTime - lastTime) / 1000;
    //cam.update(dt);
}

void init(void) {

    // Init shaders
    hairRenderShader.init();
    cumsumShader.init();
    hairDensityShader.init();
    phongShader.init();
    lightVisualizeShader.init();
    wireframeShader.init();
    voxelRender.init();
    densityShader.init();

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
    ball = util::ball(1.0, 3);
    lightBall = util::ball(0.25, 2);

    // GL stuff
    glClearColor(0.2,0.2,0.5,0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glLineWidth(1);

    // Globals
    lastTime = glutGet(GLUT_ELAPSED_TIME);	

    // Random
    voxels.resize(windowWidth * windowHeight * numLayers * 4);
}


void drawDensity() {
    hairDensityShader.use();
    glm::mat4 lightView = light.getView();
    glm::mat4 lightProj = light.getProj();
    hairDensityShader.projectionMatrix = &lightProj[0][0];
    hairDensityShader.modelViewMatrix = &lightView[0][0];
    hairDensityShader.numLayers = numLayers;
    hairDensityShader.hairLength = hairLength;
    DrawModel(ball, hairDensityShader, "in_Position", "in_Normal", NULL);
    /*
       drawModel(ball, hairDensityShader,
       &hairDensityShader.v_Position,
       &hairDensityShader.v_Normal,
       NULL);
       */
}

void drawDebugDensity() {
    densityShader.use();
    glm::mat4 transform = light.get();
    densityShader.transform = &transform[0][0];
    densityShader.numLayers = numLayers;
    DrawModel(ball, densityShader, "in_Position", NULL, NULL);
}

void cumsum() {
    cumsumShader.use();
    cumsumShader.sampler = voxelFBO.texture;
    cumsumShader.numLayers = numLayers;
    for(int i = 0; i < numLayers - 1; ++i) {
        cumsumShader.inputLayer = i + 0;
        cumsumShader.outputLayer = i + 1;
        DrawModel(quad, cumsumShader, "inPosition", NULL, "inTexCoord");
        glFinish();
    }
}

void drawBall() {
    phongShader.use();
    glm::mat4 modelViewMatrix = cam.getView();
    glm::mat4 projectionMatrix = cam.getProj();
    phongShader.modelViewMatrix = &modelViewMatrix[0][0];
    phongShader.projectionMatrix = &projectionMatrix[0][0];
    glm::vec3 lightPos = glm::mat3(modelViewMatrix) * light.getPos();
    phongShader.light = &lightPos[0];
    DrawModel(ball, phongShader, "inPosition", "inNormal", NULL);
}

void drawHair() {
    hairRenderShader.use();
    hairRenderShader.sampler = voxelFBO.texture.unit(); 
    hairRenderShader.numLayers = numLayers;
    glm::mat4 modelViewMatrix;
    glm::mat4 projectionMatrix; 
    if( currentCamera == MAIN_CAMERA ) {
        modelViewMatrix = cam.getView();
        projectionMatrix = cam.getProj();
    }
    else if( currentCamera == LIGHT_CAMERA ) {
        modelViewMatrix = light.getView();
        projectionMatrix = light.getProj();
    }
    hairRenderShader.projectionMatrix = &projectionMatrix[0][0];
    hairRenderShader.modelViewMatrix = &modelViewMatrix[0][0];
    glm::mat4 lightVP = light.get(); //lightProj * lightView; 
    hairRenderShader.lightMatrix = &lightVP[0][0];
    glm::vec3 lightPos = glm::mat3(modelViewMatrix) * light.getPos();
    hairRenderShader.light = &lightPos[0];
    hairRenderShader.hairLength = hairLength;
    hairRenderShader.shadeFactor = shadeFactor;
    //DrawModel(quad, hairRenderShader, "in_Position", "in_Normal", NULL);
    DrawModel(ball, hairRenderShader, "in_Position", "in_Normal", NULL);
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

void drawVoxels() {

    glm::mat4 lightVP = light.get();
    glm::mat4 lightVPInv = glm::inverse(lightVP);
    glm::mat4 cameraTransform = cam.get();
    glm::mat4 transform = cameraTransform * lightVPInv;

    voxelRender.use();
    voxelRender.transform = &transform[0][0];
    voxelRender.colorGain= voxelRenderColorGain;
    voxelRender.alphaGain= voxelRenderAlphaGain;
    DrawPointModel(lattice, voxelRender, "in_Position"); 
}

void displayDebug() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);

    voxelFBO.bind();
    glViewport(0,0,fboWidth,fboHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //drawDebugDensity();
    drawDensity();
    cumsum();

    voxelFBO.unbind();
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    drawVoxels();
    drawLight();
}

void displayRelease() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);

    voxelFBO.bind();
    glViewport(0,0,fboWidth,fboHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawDensity();
    cumsum();

    voxelFBO.unbind();
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.2,0.2,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    drawBall();
    drawHair();
}

void display() {

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
    glutInitContextVersion(3, 2);
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
