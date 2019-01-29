// Phong shaded bunny
// Note: Simplified! In particular, the light source is given in view
// coordinates, which means that it will follow the camera.
// You usually give light sources in world coordinates.

// Variant using the real Stanford bunny model

// 2017 version: Can switch between Phong and Gouraud

// gcc phongbunny.c ../common/*.c ../common/Linux/*.c -lGL -o phongbunny -I../common -I../common/Linux -DGL_GLEXT_PROTOTYPES  -lXt -lX11 -lm

#include "common_ext.hpp"
#include "programs/programs.hpp"

gtl::shader::HairAnimation hairAnimShader;
gtl::shader::Gouraud bunnyShader;

glm::mat4 modelViewMatrix;
glm::mat4 projectionMatrix;

glm::vec3 windForce;

GLfloat animTime = 0.0f;
GLfloat rotAngle = 0.0f;

GLsizei windowWidth = 640;
GLsizei windowHeight = 480;

Model * bunnyModel;

void init()
{
	hairAnimShader.init();
	bunnyShader.init();
        hairAnimShader.debug();
        bunnyShader.debug();

	glClearColor(0,0,0.3,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glLineWidth(1);

	bunnyModel = LoadModelPlus("models/stanford-bunny.obj");
	CenterModel(bunnyModel);
	ScaleModel(bunnyModel, 5,5,5);
	ReloadModelData(bunnyModel);

	float w = static_cast<float>(windowWidth);
	float h = static_cast<float>(windowHeight);
	projectionMatrix = glm::frustum(-w / h, w / h, -1.0f, 1.0f, 1.0f, 100.0f);
	modelViewMatrix = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	windForce = glm::vec3(0.5,0.0,0.0);
}

void updateUniforms()
{
	animTime = glutGet(GLUT_ELAPSED_TIME);
	rotAngle += 0.01;// + rotAngle;

	windForce = glm::vec3(0.8*sin(0.001*animTime),0,0.01*cos(0.01*animTime));

	glm::mat4 R = glm::rotate(rotAngle, glm::vec3(0.0f,1.0f,0.0));
	glm::mat4 viewMatrix = glm::lookAt(
					glm::vec3(0.0f, 0.0f, 2.0f),
					glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(0.0f, 1.0f, 0.0f));
	modelViewMatrix = viewMatrix * R;
}

void drawHair()
{
        hairAnimShader.use();
	hairAnimShader.projectionMatrix = &projectionMatrix[0][0];
	hairAnimShader.modelViewMatrix = &modelViewMatrix[0][0];
	hairAnimShader.windForce = &windForce[0];
	DrawModel(bunnyModel, hairAnimShader, "inPosition", "inNormal", NULL);
}

void drawBunny()
{
	bunnyShader.use();
	bunnyShader.projectionMatrix = &projectionMatrix[0][0];
	bunnyShader.modelviewMatrix = &modelViewMatrix[0][0];

	DrawModel(bunnyModel, bunnyShader, "inPosition", "inNormal", NULL);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	updateUniforms();
	drawBunny();
	drawHair();
	glutSwapBuffers();
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
	init ();
	glutMainLoop();
}
