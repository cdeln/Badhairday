
#include "common.h"
#include "programs/programs.hpp"
#include <iostream>

mat4 projectionMatrix;
Model *bunny;
gtl::shader::Gouraud gouraud;

void init(void) {

    gouraud.init(GL_TRUE);

	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	projectionMatrix = frustum(-0.5, 0.5, -0.5, 0.5, 1.0, 300.0);

	// Upload geometry to the GPU:
	bunny = LoadModelPlus("models/stanford-bunny.obj");
	CenterModel(bunny);
	ScaleModel(bunny, 5, 5, 5);
	ReloadModelData(bunny);

    gouraud.use();
    gouraud.projectionMatrix = projectionMatrix.m;
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
	
	
    gouraud.use();
    gouraud.modelviewMatrix = m.m;
    DrawModel(bunny, gouraud, "inPosition", "inNormal", "");

	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("Gouraud test");
	glutRepeatingTimerFunc(20);
	glutDisplayFunc(display); 
	init ();
	glutMainLoop();}
