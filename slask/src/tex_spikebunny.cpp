// Phong shaded bunny
// Note: Simplified! In particular, the light source is given in view
// coordinates, which means that it will follow the camera.
// You usually give light sources in world coordinates.

// Variant using the real Stanford bunny model

// 2017 version: Can switch between Phong and Gouraud

// gcc phongbunny.c ../common/*.c ../common/Linux/*.c -lGL -o phongbunny -I../common -I../common/Linux -DGL_GLEXT_PROTOTYPES  -lXt -lX11 -lm

#include "common_ext.hpp"
#include "util.hpp"
#include "programs/programs.hpp"
#include "LoadTGA.h"
#include "camera.hpp"
#include "macros.h"

gtl::shader::HairTexRender hairTexShader;
gtl::shader::Gouraud bunnyShader;

glm::mat4 modelViewMatrix;
glm::mat4 projectionMatrix;

glm::vec3 eyePos;
glm::vec3 center;
glm::vec3 upVec;

GLfloat animTime = 0.0f;
GLfloat rotAngle = 0.0f;

GLsizei windowWidth = 640;
GLsizei windowHeight = 480;

Model * dummyModel;
Model * ballModel;

glm::vec3 ballPos;

gtl::Texture<GL_TEXTURE_2D> hairTex;
GLuint hairTexUnit;

Camera cam;

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

void init()
{
	hairTexShader.init();
	bunnyShader.init();
  hairTexShader.debug();
  bunnyShader.debug();

	glClearColor(0,0,0.3,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glLineWidth(1);

	//dummyModel = LoadModelPlus("models/VirtualMan.obj");
	dummyModel = LoadModelPlus("models/longneck.obj");
	CenterModel(dummyModel);
	float scale = 2;
	ScaleModel(dummyModel, scale, scale, scale);
	ReloadModelData(dummyModel);

	ballModel = util::ball(0.2);
	ballPos = glm::vec3(0,0,0);

	LoadTGATextureSimple((char *)"textures/longneck.tga", &hairTexUnit);

	//hairTex.init(); //TODO: Jag tror att något fattas i denna konstruktorn
	hairTex.bind(hairTexUnit);

	float w = static_cast<float>(windowWidth);
	float h = static_cast<float>(windowHeight);
	float aspect = w / h;
	cam.setPerspective(45 * M_PI / 180.0, aspect, 0.01, 100);
	cam.setViewMode(Camera::ViewMode::PIVOT);
	cam.setDist(5);
}

void updateUniforms()
{
	float omega = 0.001;
	animTime = glutGet(GLUT_ELAPSED_TIME);

	modelViewMatrix = cam.getView();
	projectionMatrix = cam.getProj();

	ballPos = glm::vec3(sin(omega*animTime), 0, cos(omega*animTime));
}

void drawHair()
{
	hairTexShader.use();
	hairTexShader.projectionMatrix = &projectionMatrix[0][0];
	hairTexShader.modelViewMatrix = &modelViewMatrix[0][0];
	hairTexShader.tex = 0;
	hairTexShader.forceCenter = &ballPos[0];
	DrawModel(dummyModel, hairTexShader, "inPosition", "inNormal", "inTexCoord");
}

void drawBunny()
{
	bunnyShader.use();
	bunnyShader.projectionMatrix = &projectionMatrix[0][0];
	bunnyShader.modelviewMatrix = &modelViewMatrix[0][0];
	DrawModel(dummyModel, bunnyShader, "inPosition", "inNormal", NULL);
}

void drawBall()
{
	glm::mat4 model2world = glm::translate(ballPos);
	glm::mat4 model2view = modelViewMatrix * model2world;
	bunnyShader.use();
	bunnyShader.projectionMatrix = &projectionMatrix[0][0];
	bunnyShader.modelviewMatrix = &model2view[0][0];
	DrawModel(ballModel,bunnyShader,"inPosition", "inNormal",NULL);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	updateUniforms();
	drawBall();
	drawBunny();
	drawHair();
	glutSwapBuffers();
}

void mouseCallback(int x, int y)
{
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

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitContextVersion(3, 2);
	glutInitWindowSize (windowWidth, windowHeight);
	glutCreateWindow ("GL3 phong Stanford bunny example");
	glutRepeatingTimerFunc(20);

	glutDisplayFunc(display);
	glutMotionFunc(mouseCallback);
	glutMouseFunc(clickCallback);

	init ();
	glutMainLoop();
}
