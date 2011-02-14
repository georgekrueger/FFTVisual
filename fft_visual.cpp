#include <stdlib.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <stdio.h>

static const int Width = 512;
static const int Height = 512;

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

int RotateY = 45;

/*
 * GLUT callbacks:
 */
static void update_fade_factor(void)
{
}

//static void drawBox()
//{
//	
//}

void DrawCube()
{
	glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);	// top face
		glVertex3f(0.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);	// front face
		glVertex3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);	// right face
		glVertex3f(0.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, -1.0f, -1.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, 0.0f);	// left face
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, -1.0f, 0.0f);	// bottom face
		glVertex3f(0.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, -1.0f);	// back face
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(0.0f, -1.0f, -1.0f);
	glEnd();
}

static void SetupProjection()
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

    glutSwapBuffers();
}

// called on mouse movement 
void motion (int x, int y) 
{
	if (ButtonStates[LEFT].pressed) {
		glMatrixMode(GL_MODELVIEW);
		int dx = ButtonStates[LEFT].x - x;
		RotateY += dx;
		printf("Moved x: %d, RotateY: %d\n", dx, RotateY);
		ButtonStates[LEFT].x = x;
		ButtonStates[LEFT].y = y;
	}

  /*const double factor = 20.0; 
  bool changed = false; 

  // process UI events 
  if (buttons[GLUT_LEFT_BUTTON] == GLUT_DOWN)  
    // rotate camera orientation using left mouse button 
    camera.orbit(cur_x, cur_y, x, y, 
                 glutGet(GLUT_WINDOW_WIDTH), 
                 glutGet(GLUT_WINDOW_HEIGHT)); 
    changed = true; 
   
  if (buttons[GLUT_MIDDLE_BUTTON] == GLUT_DOWN)  
    // track camera forward and back using middle mouse button 
    camera.move(0, 0, (y - cur_y)/factor); 
    changed = true; 
   
  if (buttons[GLUT_RIGHT_BUTTON] == GLUT_DOWN)  
    // pan camera using right mouse button 
    camera.move((x - cur_x)/factor, (cur_y - y)/factor, 0); 
    changed = true; 
   

  // update everything 
  if (changed)  
    setup_view(); 
    glutPostRedisplay(); */
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

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(Width, Height);
    glutCreateWindow("Hello World");
    glutIdleFunc(&update_fade_factor);
    glutDisplayFunc(&render);
    glutMouseFunc(mouse); 
	glutMotionFunc(motion); 
    
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	SetupProjection();

    glutMainLoop();
    return 0;
}

