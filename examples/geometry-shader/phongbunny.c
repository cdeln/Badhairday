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
GLuint gouraud, phong, currentShader;
char mode = 0;

void init(void)
{
	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 300.0);

	// Load and compile shader
	phong = loadShadersG("phong.vert", "phong.frag", "phong.geom");
	gouraud = loadShaders("gouraud.vert", "gouraud.frag");
	currentShader = gouraud;
	glUseProgram(phong);
	
	// Upload geometry to the GPU:
	bunny = LoadModelPlus("stanford-bunny.obj");
	CenterModel(bunny);
	ScaleModel(bunny, 5, 5, 5);
	ReloadModelData(bunny);


	glUseProgram(phong);
	glUniformMatrix4fv(glGetUniformLocation(phong, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
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
	
	a += 0.001;
	m1 = Rz(M_PI/5);
	m2 = Ry(a);
	m = Mult(m2,m1);
	m = Mult(worldToView, m);
	
	
	glUseProgram(gouraud);
	glUniformMatrix4fv(glGetUniformLocation(gouraud, "modelviewMatrix"), 1, GL_TRUE, m.m);
        DrawModel(bunny, gouraud, "inPosition", "inNormal", "");

	glUseProgram(phong);
	glUniformMatrix4fv(glGetUniformLocation(phong, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(bunny, phong, "inPosition", "inNormal", "");	
	
	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	mode = !mode;
	if (mode) 
		currentShader = phong;
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
