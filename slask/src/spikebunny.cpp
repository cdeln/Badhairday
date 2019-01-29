// Phong shaded bunny
// Note: Simplified! In particular, the light source is given in view
// coordinates, which means that it will follow the camera.
// You usually give light sources in world coordinates.

// Variant using the real Stanford bunny model

// 2017 version: Can switch between Phong and Gouraud

// gcc phongbunny.c ../common/*.c ../common/Linux/*.c -lGL -o phongbunny -I../common -I../common/Linux -DGL_GLEXT_PROTOTYPES  -lXt -lX11 -lm

#include "MicroGlut.h"
//uses framework Cocoa
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"


mat4 projectionMatrix;

// Pointer to model data
Model *bunny;

// Reference to shader program
GLuint gouraud, hair, currentShader;
char mode = 0;

void init(void)
{
	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 300.0);

	// Load and compile shader
	hair = loadShadersG("shaders/minimal.vert", "shaders/minimal.frag", "shaders/tagg.geom");
	gouraud = loadShaders("shaders/gouraud.vert", "shaders/gouraud.frag");
	currentShader = gouraud;
	glUseProgram(hair);
	
	// Upload geometry to the GPU:
	bunny = LoadModelPlus("models/stanford-bunny.obj");
	CenterModel(bunny);
	ScaleModel(bunny, 5, 5, 5);
	ReloadModelData(bunny);


	glUseProgram(hair);
	glUniformMatrix4fv(glGetUniformLocation(hair, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(gouraud);
	glUniformMatrix4fv(glGetUniformLocation(gouraud, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

GLfloat a = 0.0;

void display(void)
{
	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	mat4 worldToView, m1, m2, m, tr, scale;
	worldToView = lookAt(0,0,2, 0,0,0, 0,1,0);
	float t = glutGet(GLUT_ELAPSED_TIME);	
	a += 0.001;
	m1 = Rz(M_PI/5);
	m2 = Ry(a);
	m = Mult(m2,m1);
	m = Mult(worldToView, m);
	
	
	glUseProgram(gouraud);
	glUniformMatrix4fv(glGetUniformLocation(gouraud, "modelviewMatrix"), 1, GL_TRUE, m.m);
        DrawModel(bunny, gouraud, "inPosition", "inNormal", "");

	glUseProgram(hair);
	glUniformMatrix4fv(glGetUniformLocation(hair, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(bunny, hair , "inPosition", "inNormal", "");	
	glUniform1f(glGetUniformLocation(hair,"t"), t);
	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	mode = !mode;
	if (mode) 
		currentShader = hair;
	else
		currentShader = gouraud;

}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("GL3 phong Stanford bunny example");
	glutRepeatingTimerFunc(20);
	glutDisplayFunc(display); 
	glutKeyboardFunc(keys);
	init ();
	glutMainLoop();}
