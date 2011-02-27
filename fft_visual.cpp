#include <stdlib.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <stdio.h>

#include "../PBRSynth/Lib/wavfile.h"
#include "Audio/Audio.h"

static const int Width = 512;
static const int Height = 512;
int Time = 0;

typedef enum
{
	LEFT,
	RIGHT,
	MIDDLE
} BUTTON;

struct BUTTON_STATE {
	bool pressed;
	int x;
	int y;
} ButtonStates[3];

float RotateY = 0;
float RotateX = 0;
float TranslateX = 0;
float CubeHeight = 0;

/*
 * GLUT callbacks:
 */
static void update(void)
{
	int ElapsedTime = 0;
	if (Time > 0) {
		ElapsedTime = glutGet(GLUT_ELAPSED_TIME) - Time;
	}
	Time = glutGet(GLUT_ELAPSED_TIME);
	
	CubeHeight += ElapsedTime * 0.001;
	if (CubeHeight >= 1) CubeHeight = 0;
	
	glutPostRedisplay();
}

static void DrawCube()
{
	glBegin(GL_QUADS);
		glVertex3f(0.0f, CubeHeight, 0.0f);	// top face
		glVertex3f(0.0f, CubeHeight, -1.0f);
		glVertex3f(-1.0f, CubeHeight, -1.0f);
		glVertex3f(-1.0f, CubeHeight, 0.0f);
		glVertex3f(0.0f, CubeHeight, 0.0f);	// front face
		glVertex3f(-1.0f, CubeHeight, 0.0f);
		glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, CubeHeight, 0.0f);	// right face
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glVertex3f(0.0f, CubeHeight, -1.0f);
		glVertex3f(-1.0f, CubeHeight, 0.0f);	// left face
		glVertex3f(-1.0f, CubeHeight, -1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);	// bottom face
		glVertex3f(0.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, CubeHeight, -1.0f);	// back face
		glVertex3f(-1.0f, CubeHeight, -1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
	glEnd();
}

static void DrawAxis()
{
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(8, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 8, 0);
	glColor3f(0, 0, 8);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 8);
	glEnd();
}

static void UpdateProjection()
{
	glViewport(0, 0, Width, Height);		// reset the viewport to new dimensions
	
	glMatrixMode(GL_PROJECTION);			// set projection matrix current matrix
	glLoadIdentity();						// reset projection matrix

	// calculate aspect ratio of window
	gluPerspective(52.0f,(GLfloat)Width/(GLfloat)Height,1.0f,1000.0f);
}

static void render(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(0.0, 0.0, 0.0);
    
    glMatrixMode(GL_MODELVIEW);				// set modelview matrix
	glLoadIdentity();						// reset modelview matrix
	
    glTranslatef(0, 0, -10);
   	glRotatef(RotateY, 0.0, 1.0, 0);
	glRotatef(RotateX, 1.0, 0, 0);
	glTranslatef(0.5, 0.5, 0.5);
   
    //glPointSize(5.0);
    //glEnable(GL_POINT_SMOOTH);
    //glBegin(GL_POINTS);
    //	glVertex3f(0.0, 0.0, 0.0);
    //glEnd();
    
    //glBegin(GL_LINES);
    //	glVertex3f(-2.0, -1.0, 0.0);
    //	glVertex3f(3.0, 1.0, 0.0);
    //glEnd();
    
    DrawCube();
	DrawAxis();

    glutSwapBuffers();
}

// called on mouse movement 
void motion (int x, int y) 
{
	if (ButtonStates[LEFT].pressed) {
		glMatrixMode(GL_MODELVIEW);
		int dx = ButtonStates[LEFT].x - x;
		int dy = ButtonStates[LEFT].y - y;
		RotateY += (dx * -0.5);
		if (RotateY >= 360) RotateY -= 360;
		RotateX += (dy * -0.5);
		if (RotateX >= 360) RotateX -= 360;
		printf("Moved x: %d, RotateY: %f, TranslateX: %f\n", dx, RotateY, TranslateX);
		ButtonStates[LEFT].x = x;
		ButtonStates[LEFT].y = y;
	}
}   


void mouse (int button, int state, int x, int y) 
{
	if (button == GLUT_LEFT_BUTTON) {
		ButtonStates[LEFT].x = x;
		ButtonStates[LEFT].y = y;
		if (state == GLUT_DOWN) {
			ButtonStates[LEFT].pressed = true;
			printf("Left button pressed down\n");
		}
		else {
			ButtonStates[LEFT].pressed = false;
			printf("Left button released\n");
		}
	}
}


/*
 * Entry point
 */
int main(int argc, char** argv)
{
	printf("main\n");
	
	Audio_Init(44100, 64);
	AudioHandle Handle;
	Audio_LoadFile("../../../SynthGuitar.wav", &Handle);
	Audio_PlayHandle(&Handle, 1.0);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(Width, Height);
    glutCreateWindow("Hello World");
    glutIdleFunc(&update);
    glutDisplayFunc(&render);
    glutMouseFunc(mouse); 
	glutMotionFunc(motion); 
    
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	UpdateProjection();

    glutMainLoop();
	
	Audio_Deinit();
    return 0;
}

